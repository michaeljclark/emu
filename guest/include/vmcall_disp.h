#pragma once

#ifndef __vmarg
#define __vmarg(X) ((long) (X))
#endif

#define __vmcall1(n,a) __vmcall1(n,__vmarg(a))
#define __vmcall2(n,a,b) __vmcall2(n,__vmarg(a),__vmarg(b))
#define __vmcall3(n,a,b,c) __vmcall3(n,__vmarg(a),__vmarg(b),__vmarg(c))
#define __vmcall4(n,a,b,c,d) __vmcall4(n,__vmarg(a),__vmarg(b),__vmarg(c),__vmarg(d))
#define __vmcall5(n,a,b,c,d,e) __vmcall5(n,__vmarg(a),__vmarg(b),__vmarg(c),__vmarg(d),__vmarg(e))
#define __vmcall6(n,a,b,c,d,e,f) __vmcall6(n,__vmarg(a),__vmarg(b),__vmarg(c),__vmarg(d),__vmarg(e),__vmarg(f))
#define __vmcall7(n,a,b,c,d,e,f,g) __vmcall7(n,__vmarg(a),__vmarg(b),__vmarg(c),__vmarg(d),__vmarg(e),__vmarg(f),__vmarg(g))

#define __VMCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __VMCALL_NARGS(...) __VMCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __VMCALL_CONCAT_X(a,b) a##b
#define __VMCALL_CONCAT(a,b) __VMCALL_CONCAT_X(a,b)
#define __VMCALL_DISP(b,...) __VMCALL_CONCAT(b,__VMCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __vmcall(...) __VMCALL_DISP(__vmcall,__VA_ARGS__)
