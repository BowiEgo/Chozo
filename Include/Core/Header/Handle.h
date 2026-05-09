#pragma once

namespace CZ {

template <typename TObject> class Handle {
public:
    Handle() = default;
    Handle(TObject* obj) : m_Obj(obj) {}

    operator bool() const { return m_Obj != nullptr; }
    operator TObject*() { return m_Obj; }
    operator const TObject*() const { return m_Obj; }
    inline TObject* Unwrap() { return m_Obj; }

protected:
    TObject* m_Obj;
};

} // namespace CZ