#include <cmath>
#include <memory>
#include <utility>
#include "DragAndDrop.hpp"
#include "DragPayload.hpp"
#include "psychic-ui.hpp"

namespace psychic_ui {

    const DragPayload *DragAndDrop::payload() const {
        return _state == State::Dragging && _payload ? &*_payload : nullptr;
    }

    std::shared_ptr<Div> DragAndDrop::source() const {
        return _state == State::Idle ? nullptr : _source.lock();
    }

    std::shared_ptr<Div> DragAndDrop::target() const {
        return _state == State::Dragging ? _target.lock() : nullptr;
    }

    void DragAndDrop::setTarget(const std::weak_ptr<Div> &target) {
        if (_state == State::Dragging) {
            _target = target;
        }
    }

    void DragAndDrop::press(const std::weak_ptr<Div> &source, const MouseEvent &press, DragSourceFn sourceFn) {
        if (_state == State::Dragging) {
            return;
        }
        _state     = State::Armed;
        _source    = source;
        _sourceFn  = std::move(sourceFn);
        _press     = press;
        _payload.reset();
        _inputMode = press.source;
        _windowX   = press.windowX;
        _windowY   = press.windowY;
    }

    bool DragAndDrop::move(const float windowX, const float windowY) {
        switch (_state) {
            case State::Idle:
                return false;
            case State::Armed:
                _windowX = windowX;
                _windowY = windowY;
                if (std::hypot(windowX - _press.windowX, windowY - _press.windowY) <= dragThreshold) {
                    // Drops a source removed while armed, so that it can't start later
                    if (_source.expired()) {
                        reset();
                    }
                    return false;
                }
                return start();
            case State::Dragging:
                if (_source.expired()) {
                    cancel();
                    return false;
                }
                _windowX = windowX;
                _windowY = windowY;
                return true;
        }
        return false;
    }

    bool DragAndDrop::start() {
        if (_state != State::Armed) {
            return false;
        }
        if (_source.expired() || !_sourceFn) {
            reset();
            return false;
        }
        // Called from copies: the source's function may cancel the session or remove the source
        const auto       sourceFn = std::move(_sourceFn);
        _sourceFn                   = nullptr;
        const MouseEvent press    = _press;
        auto             payload  = sourceFn(press);
        if (_state != State::Armed) {
            return false;
        }
        if (!payload || _source.expired()) {
            reset();
            return false;
        }
        _payload = std::move(payload);
        _state   = State::Dragging;
        return true;
    }

    bool DragAndDrop::release(const float windowX, const float windowY) {
        const bool dragging = _state == State::Dragging;
        _windowX            = windowX;
        _windowY            = windowY;
        reset();
        return dragging;
    }

    void DragAndDrop::cancel() {
        reset();
    }

    void DragAndDrop::reset() {
        _state = State::Idle;
        _source.reset();
        _target.reset();
        _sourceFn = nullptr;
        _payload.reset();
    }

}
