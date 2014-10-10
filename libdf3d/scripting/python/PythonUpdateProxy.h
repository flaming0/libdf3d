#pragma once

namespace df3d { namespace scripting {

class ScriptManager;

#if DF3D_USES_PYTHON

class DF3D_DLL PythonUpdateProxy : boost::noncopyable
{
    friend class ScriptManager;

    struct Impl;
    scoped_ptr<Impl> m_pImpl;

    PythonUpdateProxy(const char *pyFile, const char *pyFunction, ScriptManager *sm);

public:
    ~PythonUpdateProxy();

    void update(float dt);
};

#endif

} }