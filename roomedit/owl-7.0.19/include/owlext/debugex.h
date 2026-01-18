/***************************************************************************
  OWL Extensions (OWLEXT) Class Library
    DebugEx.h

  This header declares several macros for debugging purposes.  There are
    two general categories:  ASSERT and TRACE.  So as not to conflict with
    OWL's TRACE macros, this module uses CTRACE instead.  Below is a run
    down on the macros and briefly how they are used:

    IFDEBUG(X)  X is expanded only for debug builds.  Useful for a
                parameter that is only ASSERTed on.  Eg:

                            void foo (int IFDEBUG(x))
                            {
                                ASSERT (x);
                            }

    VALIDPTR(P) Returns TRUE for valid pointers (not all non-null ptrs
                are valid).  The release build simply tests for null.

    ASSERT(B)   if B is false, you get an Abort/Retry/Ignore msgbox.
                Retry drops you into the debugger at the point of the
                ASSERT statement.  For release builds, the entire
                overhead for ASSERT is removed (so it better not have
                any side-effects).

    VERIFY(B)   Same as ASSERT except that the expression remains for
                release builds.  This is ideal for an expression that
                has side-effects.  Eg: VERIFY(LoadString(...)).

    DBGBRK()    Embeds a debugger breakpoint.

    COMPILE_ASSERT(B)   Evaluates a compile-time expression and fails
                to compile if B is false.  Useful for array size tests
                and such.  Eg:

                    COMPILE_ASSERT (ARRAYCOUNT(myTable) == IDX_LAST);

                #define ARRAYCOUNT(A)   (sizeof(A) / sizeof(A[0]))


    Tracing is built using trace groups and trace levels.  A trace group
    is used to bind common pieces together.  For example, a class will
    usually define a trace group that each of its methods use for its
    trace messages.  A trace group is simply a global object.  There are
    macros for defining one, declaring a group as extern, and setting
    the default group for a source file.  Each group object has a current
    trace level member.

    The trace level is an integer that is used as a filter.  All trace
    messages include a trace level.  If the level of the message is <= to
    the current trace level for its group, the message will generate
    output.  By convention, there are three predefined levels: traceError
    (level 1), traceWarning (level 5) and traceInfo (level 10).  This
    allows all traces to be turned off for a group by setting its level
    to 0.

    The trace levels for all groups in a module (EXE or DLL) are stored in
    an INI file by the name of "MODULE.TRC" that resides in the same
    directory as the module.  This file is read when CTRACE_INIT is called
    in that module.  The HINSTANCE is needed to locate the BIN directory.

    DEFINE_CTRACEGROUP(name,defLevel)  Instantiates a trace group object
                callled 'name' + 'TraceGroup'.  Eg: ListBoxTest would be
                ListBoxTestTraceGroup.  The defLevel is the level for
                which traces will be generated.  Typically, the value
                traceWarning is used for this.  The level in the TRC file
                takes precedence.

    EXTERN_CTRACEGROUP(name)  Declares that a trace group called 'name'
                as an extern group.

    DEFAULT_CTRACEGROUP(name)  Declares that the default trace group for
                this file is 'name' (same name as used in DEFINE_ above).
                This is required to use CTRACE instead of CTRACEEX (see
                below).

    CTRACEEX(name,level,(...))   Generates a trace at 'level' for the
                trace group labeld 'name'.  The syntax is a bit funny
                because it was the only way to remove all overhead for
                the trace in a non-trace build.  Eg:

                  CTRACEEX (ListBoxTest, traceWarning,
                                         ("Bad thing %d", badThing));

    CTRACE(level,(...))  Similar to CTRACEEX, except that the group is
                the "default" group.

    CTRACEEX_IF(b,name,level,(...))  This macro is used for a conditional
                trace.  If b is true, the result is equivalent to CTRACEEX.

    CTRACE_IF(b,level,(...))  Similar to CTRACEEX_IF, but uses the default
                trace group.

    ASSERT_TRACE(B,(...))  Similar to CTRACE_IF, except that we see the msg
                in the assert msgbox and also as if it were a normal trace.

    VERIFY_TRACE(B,(...))  Same difference as ASSERT vs. VERIFY

    There is code in the TApp class that adds "View Trace" to the system
    menu of the main window.  When selected, it displays a TTraceWindow for
    the traces (the default traces go to OutputDebugString).  The trace
    window can save the trace buffer, set the font, modify the trace levels
    on-the-fly, etc..  This is useful for testers who might not have a debug
    monitor or DbWin.  All of that is built using the functionality provided
    by this module--there is no knowledge of any GUI stuff down here other
    than MessageBox.

***************************************************************************/
#if !defined (__OWLEXT_DEBUGEX_H)
#define __OWLEXT_DEBUGEX_H

#  ifdef _DEBUG
#    include <dos.h>  // __emit__ prototype for debugger breakpoint
#  else
#   ifndef NDEBUG
#      define NDEBUG
#   endif
# endif

# define  IFDEBUG            IFDBG
# define  IFNDEBUG           IFNDBG

# define  DBGBRK()           IFDEBUG (OWL_BREAK) // by Yura Bidus 05/31/98

// use COUNTOF(a) insteed, see owl/private/defs.h
# ifndef ARRAYCOUNT
#    define ARRAYCOUNT(A)   COUNTOF(A)
#  endif

  //  This guy generates a compiler error such as
  //          'Array must have at least one element'
  //  if the boolean condition argument is false:
# define COMPILE_ASSERT(B)   typedef char _Assert##__LINE__ [2*(!!(B)) - 1]

# ifdef _DEBUG

    //  If we don't turn these on, frequent use of ASSERT will fill DGROUP:
    //
#    pragma option -d  // merge duplicate strings ON
#    ifndef __WIN32__
#      pragma option -dc // constant strings in code segments ON
#    endif

#    define  ASSERT(X)  \
                ( (!(X) && DebugEx::AssertFailure (__FILE__,__LINE__)) \
                        ? DBGBRK() : (void)0 )

#    if !defined(VERIFY)
#      define  VERIFY(X)  ASSERT(X)
#    endif
#  else

#   define  ASSERT(X)  ((void) 0)
#    if !defined(VERIFY)
#      define  VERIFY(X)  ((void) (X))
#    endif

# endif

  //==========================================================================

#    define  _TRACEMETHOD       __stdcall
#    define  _TRACEMETHODIMP    _TRACEMETHOD
#    define  _TRACEINTF

#  define  ASSERT_CTRACE(X,msg)       ASSERT_CTRACEEX (X, __def, msg)
#  define  VERIFY_CTRACE(X,msg)       VERIFY_CTRACEEX (X, __def, msg)

#  ifndef _DEBUG
#    define  CTRACE_INIT(hInst)
#    define  DEFINE_CTRACEGROUP(name,level)
#    define  EXTERN_CTRACEGROUP(name)
#    define  DEFAULT_CTRACEGROUP(name)
#    define  DEFINE_DEFAULT_CTRACEGROUP(name,level)
#    define  EXTERN_DEFAULT_CTRACEGROUP(name)

#    define  CTRACEEX(name,level,msg)
#    define  CTRACE(level,msg)
#    define  CTRACEEX_IF(b,name,level,msg)
#    define  CTRACE_IF(b,level,msg)

#    define  ASSERT_CTRACEEX(X,name,msg)     ASSERT (X)
#    define  VERIFY_CTRACEEX(X,name,msg)     VERIFY (X)

#  else  // _DEBUG (look to end of file)

namespace DebugEx {
#  define DEBUGEXT DebugEx

    // Generic definitions/compiler options (eg. alignment) preceding the
    // definition of classes
    //
#    include <owl/preclass.h>

#    define  ASSERT_CTRACEEX(X,name,msg) \
            ( (!(X) && DEBUGEXT::AssertFailure (__FILE__, __LINE__, \
                                                & CTRACEGROUP(name), \
                                                DEBUGEXT::TraceF msg)) \
                    ? DBGBRK() : (void)0 )

#    define  VERIFY_CTRACEEX(X,name,msg) \
            ASSERT_CTRACEEX(X,name,msg)

#    define CTRACE_INIT(hInst)          DEBUGEXT::traceLink.Init (hInst)

    struct _TRACEINTF ITraceLink;

    struct _TRACEINTF ITraceGroup
    {
        virtual unsigned        _TRACEMETHOD Level () = 0;
        virtual void            _TRACEMETHOD Level (unsigned) = 0;
        virtual ITraceLink *    _TRACEMETHOD Link () = 0;
        virtual const char *    _TRACEMETHOD Name () = 0;
        virtual ITraceGroup *   _TRACEMETHOD Next () = 0;
    };

    class _TRACEINTF TTraceGroup : public ITraceGroup
    {
    public:
        TTraceGroup (const char *name, unsigned level);

        // return true to ignore trace message, false for normal execution.
        typedef bool (* TTraceHook) (TTraceGroup *, unsigned level,
                                                    const char *msg);

        static TTraceHook   TraceHook   ()              { return sTraceHook; }
        static void         TraceHook   (TTraceHook fn) { sTraceHook = fn; }

        void                Trace (unsigned level, const char *msg);

        //  Group navigation:
        //
        static TTraceGroup *FirstGroup () { return sFirst; }
        TTraceGroup        *NextGroup() { return mNext; }
        void                ReadState (const char * iniFile);
        void                SaveState (const char * iniFile);

        //  ITraceGroup interface:
        //
        virtual unsigned        _TRACEMETHODIMP Level ();
        virtual void            _TRACEMETHODIMP Level (unsigned);
        virtual ITraceLink *    _TRACEMETHODIMP Link ();
        virtual const char *    _TRACEMETHODIMP Name ();
        virtual ITraceGroup *   _TRACEMETHODIMP Next ();

    private:
        static  TTraceGroup   * sFirst;
        static  TTraceHook      sTraceHook;

        TTraceGroup   * mNext;
        const char    * mName;
        unsigned        mLevel; // current trace level for group (0 = all)
    };

    //  Mangles the user-visible group name into something less likely to cause
    //  duplicate symbol errors:
    //
#    define  CTRACEGROUP(name)       name##TraceGroup

    //  Used in a file to declare a trace group.  The trace group is a named
    //  entity that is used to associate common activities.  Eg, a class may be
    //  implemented such that any trace messages it outputs fall into one group
    //  which may be activated as desired via an INI file.  This name can then
    //  be used in a CTRACEEX statement.
    //
#    define DEFINE_CTRACEGROUP(name,level)    \
            DEBUGEXT::TTraceGroup CTRACEGROUP(name) (#name, level);

    //  Declares an extern trace group.  This is used to write traces as a member
    //  of a group from a file other than the file where DEFINE_CTRACEGROUP(name)
    //  was used.  This name can then be used in a CTRACEEX statement.
    //
#    define EXTERN_CTRACEGROUP(name)         \
            extern DEBUGEXT::TTraceGroup    CTRACEGROUP(name);

    //  This macro is used to declare a default trace group for a file.  This is
    //  required to use the simpler CTRACE macro.
    //
#    define DEFAULT_CTRACEGROUP(name)   \
            static DEBUGEXT::TTraceGroup & CTRACEGROUP(__def) = CTRACEGROUP(name); \
            static void __KillNoUseDefTraceGroupWarning (DEBUGEXT::TTraceGroup &) \
                    { __KillNoUseDefTraceGroupWarning (CTRACEGROUP(__def)); }

    //  This is the typically used macro, where we both DEFINE a new trace group
    //  and use it as the DEFAULT trace group for this file.
    //
#    define DEFINE_DEFAULT_CTRACEGROUP(name,level) \
            DEFINE_CTRACEGROUP (name, level) \
            DEFAULT_CTRACEGROUP (name)

#    define EXTERN_DEFAULT_CTRACEGROUP(name) \
            EXTERN_CTRACEGROUP (name) \
            DEFAULT_CTRACEGROUP (name)

    extern const char * __cdecl TraceF (const char *fmt, ...);

    enum TTraceLevel  // common values used for trace levels (convention only)
    {
        traceError     = 1,
        traceWarning   = 5,
        traceInfo      = 10
    };

    //  Examples:
    //      CTRACEEX (GroupName, 3 /* level */, ("Message"));
    //      CTRACEEX (GroupName, 2 /* level */, ("Value %d", nValue));
    //      CTRACE (2 /* level */, ("Default trace message %d", value));
    //
#    define CTRACEEX(name,level,msg) CTRACEGROUP(name).Trace (level, DEBUGEXT::TraceF msg)
#    define CTRACE(level,msg)        CTRACEEX (__def, level, msg)

    //
    //  CTRACE_IF (i < 10, traceWarning, ("Value of i (= %d) is too low", i));
    //
#    define CTRACEEX_IF(b,name,level,msg)   \
                (!(b) ? (void)0 : CTRACEEX(name,level,msg))
#    define CTRACE_IF(b,level,msg)   \
                CTRACEEX_IF (b, __def, level, msg)

    //--------------------------------------------------------------------------
    //  In a multimodule system, one module needs to be in charge of tracing.
    //  To do that, they need to link up with each other.  And to do that, one
    //  module must be designated as the root of the tracing hierarchy.  That
    //  module will be told about all traces in all other modules.  It also has
    //  any global state information that is needed by all modules (such as the
    //  indent level, active status, etc.).
    //
    //  The root module needs to link up with some of the other modules in the
    //  system.  The easiest way to do that is to call a function in each of
    //  those modules, passing an ITraceLink* to it:
    //
    //      void Mod1SetTraceLink (ITraceLink * link)
    //      {
    //          link->AddChild (& traceLink);
    //      }
    //
    //  If Mod1 knows about lower level modules, it could initialize them as
    //  well:
    //
    //      void Mod1SetTraceLink (ITraceLink * link)
    //      {
    //          link->AddChild (& traceLink);
    //
    //          Mod2SetTraceLink (& traceLink);
    //          Mod3SetTraceLink (& traceLink);
    //      }
    //
    //  This will link Mod2 and Mod3 as children of Mod1 and Mod1 as a child
    //  of whomever called Mod1SetTraceLink.
    //

    struct _TRACEINTF ITraceLink
    {
        virtual bool            _TRACEMETHOD Active            () = 0;
        virtual void            _TRACEMETHOD AddChild          (ITraceLink *) = 0;
        virtual ITraceLink *    _TRACEMETHOD FirstChild        () = 0;
        virtual ITraceGroup *   _TRACEMETHOD FirstGroup        () = 0;
        virtual const char *    _TRACEMETHOD Module            () = 0;
        virtual void            _TRACEMETHOD Next              (ITraceLink *) = 0;
        virtual ITraceLink *    _TRACEMETHOD Next              () = 0;
        virtual void            _TRACEMETHOD ReloadTraceLevels () = 0;
        virtual void            _TRACEMETHOD RemoveChild       (ITraceLink *) = 0;
        virtual void            _TRACEMETHOD Parent            (ITraceLink *) = 0;
        virtual ITraceLink *    _TRACEMETHOD Parent            () = 0;
        virtual void            _TRACEMETHOD SaveTraceLevels   () = 0;
        virtual void            _TRACEMETHOD TraceIncIndent    (int inc) = 0;  // 1 or -1
        virtual int             _TRACEMETHOD TraceIndent       () = 0;
        virtual int             _TRACEMETHOD TraceIndentSize   () = 0;
        virtual void            _TRACEMETHOD TraceMessage      (const char *, ITraceLink *) = 0;
    };

    //  Don't pass TTraceLink*'s to other modules:
    class _TRACEINTF TTraceLink : public ITraceLink
    {
    public:
        TTraceLink ();
        ~TTraceLink ();

        virtual bool          _TRACEMETHODIMP Active            ();
        virtual void          _TRACEMETHODIMP AddChild          (ITraceLink *);
        virtual ITraceLink *  _TRACEMETHODIMP FirstChild        ();
        virtual ITraceGroup * _TRACEMETHODIMP FirstGroup        ();
        virtual const char *  _TRACEMETHODIMP Module            ();
        virtual void          _TRACEMETHODIMP Next              (ITraceLink *);
        virtual ITraceLink *  _TRACEMETHODIMP Next              ();
        virtual void          _TRACEMETHODIMP ReloadTraceLevels ();
        virtual void          _TRACEMETHODIMP RemoveChild       (ITraceLink *);
        virtual void          _TRACEMETHODIMP Parent            (ITraceLink *);
        virtual ITraceLink *  _TRACEMETHODIMP Parent            ();
        virtual void          _TRACEMETHODIMP SaveTraceLevels   ();
        virtual void          _TRACEMETHODIMP TraceIncIndent    (int inc);
        virtual int           _TRACEMETHODIMP TraceIndent       ();
        virtual int           _TRACEMETHODIMP TraceIndentSize   ();
        virtual void          _TRACEMETHODIMP TraceMessage  (const char *msg,
                                                            ITraceLink *source = 0);

        typedef void (* TTraceProc) (ITraceLink *source, const char *msg);

        void        Active          (bool b)        { mActive = b; }
        void        Init            (HINSTANCE);
        TTraceProc  TraceProc       ()              { return mTraceProc; }
        void        TraceProc       (TTraceProc fn) { mTraceProc =
                                                        fn ? fn : DefTrace; }
        void        TraceIndentSize (int size)      { mIndentSize = size; }

    private:
        TTraceProc      mTraceProc;
        HINSTANCE       mInstance;
        ITraceLink    * mParent;
        ITraceLink    * mFirstChild;
        ITraceLink    * mNext;
        bool            mActive;
        int             mIndent;
        int             mIndentSize;
        char            mModule [16];

        static void DefTrace (ITraceLink *source, const char *msg);
                      // ==> OutputDebugString

        void TraceFileName (char *);
    };

    extern TTraceLink   traceLink;

    extern bool __cdecl AssertFailure (const char * file, unsigned line,
                                   TTraceGroup * = 0, const char *msg = 0);

    // Generic definitions/compiler options (eg. alignment) following the
    // definition of classes
#    include <owl/posclass.h>

};

    using namespace DebugEx;

#  endif  // _DEBUG

#endif  // __DEBUGEX_H

