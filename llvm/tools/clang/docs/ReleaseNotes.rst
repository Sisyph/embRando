=====================================
Clang 3.8 (In-Progress) Release Notes
=====================================

.. contents::
   :local:
   :depth: 2

Written by the `LLVM Team <http://llvm.org/>`_

.. warning::

   These are in-progress notes for the upcoming Clang 3.8 release. You may
   prefer the `Clang 3.7 Release Notes
   <http://llvm.org/releases/3.7.0/tools/clang/docs/ReleaseNotes.html>`_.

Introduction
============

This document contains the release notes for the Clang C/C++/Objective-C
frontend, part of the LLVM Compiler Infrastructure, release 3.8. Here we
describe the status of Clang in some detail, including major
improvements from the previous release and new feature work. For the
general LLVM release notes, see `the LLVM
documentation <http://llvm.org/docs/ReleaseNotes.html>`_. All LLVM
releases may be downloaded from the `LLVM releases web
site <http://llvm.org/releases/>`_.

For more information about Clang or LLVM, including information about
the latest release, please check out the main please see the `Clang Web
Site <http://clang.llvm.org>`_ or the `LLVM Web
Site <http://llvm.org>`_.

Note that if you are reading this file from a Subversion checkout or the
main Clang web page, this document applies to the *next* release, not
the current one. To see the release notes for a specific release, please
see the `releases page <http://llvm.org/releases/>`_.

What's New in Clang 3.8?
========================

Some of the major new features and improvements to Clang are listed
here. Generic improvements to Clang as a whole or to its underlying
infrastructure are described first, followed by language-specific
sections with improvements to Clang's support for those languages.

Major New Features
------------------

- Feature1...

Improvements to Clang's diagnostics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Clang's diagnostics are constantly being improved to catch more issues,
explain them more clearly, and provide more accurate source information
about them. The improvements since the 3.7 release include:

-  ...

New Compiler Flags
------------------

The option ....


New Pragmas in Clang
-----------------------

Clang now supports the ...

Windows Support
---------------

Clang's support for building native Windows programs ...


C Language Changes in Clang
---------------------------

...

C11 Feature Support
^^^^^^^^^^^^^^^^^^^

...

C++ Language Changes in Clang
-----------------------------

- ...

C++11 Feature Support
^^^^^^^^^^^^^^^^^^^^^

...

Objective-C Language Changes in Clang
-------------------------------------

...

OpenCL C Language Changes in Clang
----------------------------------

...

Internal API Changes
--------------------

These are major API changes that have happened since the 3.7 release of
Clang. If upgrading an external codebase that uses Clang as a library,
this section should help get you past the largest hurdles of upgrading.

-  ...

AST Matchers
------------
The AST matcher functions were renamed to reflect the exact AST node names,
which is a breaking change to AST matching code. The following matchers were
affected:

=======================	============================
Previous Matcher Name	New Matcher Name
=======================	============================
recordDecl		recordDecl and cxxRecordDecl
ctorInitializer		cxxCtorInitializer
constructorDecl		cxxConstructorDecl
destructorDecl		cxxDestructorDecl
methodDecl		cxxMethodDecl
conversionDecl		cxxConversionDecl
memberCallExpr		cxxMemberCallExpr
constructExpr		cxxConstructExpr
unresolvedConstructExpr	cxxUnresolvedConstructExpr
thisExpr		cxxThisExpr
bindTemporaryExpr	cxxBindTemporaryExpr
newExpr			cxxNewExpr
deleteExpr		cxxDeleteExpr
defaultArgExpr		cxxDefaultArgExpr
operatorCallExpr	cxxOperatorCallExpr
forRangeStmt		cxxForRangeStmt
catchStmt		cxxCatchStmt
tryStmt			cxxTryStmt
throwExpr		cxxThrowExpr
boolLiteral		cxxBoolLiteral
nullPtrLiteralExpr	cxxNullPtrLiteralExpr
reinterpretCastExpr	cxxReinterpretCastExpr
staticCastExpr		cxxStaticCastExpr
dynamicCastExpr		cxxDynamicCastExpr
constCastExpr		cxxConstCastExpr
functionalCastExpr	cxxFunctionalCastExpr
temporaryObjectExpr	cxxTemporaryObjectExpr
CUDAKernalCallExpr	cudaKernelCallExpr
=======================	============================

recordDecl() previously matched AST nodes of type CXXRecordDecl, but now
matches AST nodes of type RecordDecl. If a CXXRecordDecl is required, use the
cxxRecordDecl() matcher instead.

...

libclang
--------

...

Static Analyzer
---------------

...

Core Analysis Improvements
==========================

- ...

New Issues Found
================

- ...

Python Binding Changes
----------------------

The following methods have been added:

-  ...

Significant Known Problems
==========================

Additional Information
======================

A wide variety of additional information is available on the `Clang web
page <http://clang.llvm.org/>`_. The web page contains versions of the
API documentation which are up-to-date with the Subversion version of
the source code. You can access versions of these documents specific to
this release by going into the "``clang/docs/``" directory in the Clang
tree.

If you have any questions or comments about Clang, please feel free to
contact us via the `mailing
list <http://lists.llvm.org/mailman/listinfo/cfe-dev>`_.
