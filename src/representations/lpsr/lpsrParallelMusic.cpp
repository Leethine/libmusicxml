/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include "lpsrParallelMusic.h"

#include "lpsrComments.h"

#include "lpsrVariables.h"

#include "lpsrBarNumbers.h"

#include "lpsrVoices.h"
#include "lpsrLyrics.h"

#include "lpsrStaves.h"

#include "lpsrContexts.h"

#include "oahOah.h"

#include "lpsrOah.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_lpsrParallelMusicBLock lpsrParallelMusicBLock::create (
  int                       inputLineNumber,
  lpsrElementsSeparatorKind elementsSeparatorKind)
{
  lpsrParallelMusicBLock* o =
    new lpsrParallelMusicBLock (
      inputLineNumber,
      elementsSeparatorKind);
  assert (o != nullptr);
  return o;
}

lpsrParallelMusicBLock::lpsrParallelMusicBLock (
  int                       inputLineNumber,
  lpsrElementsSeparatorKind elementsSeparatorKind)
    : lpsrElement (inputLineNumber)
{
  fElementsSeparatorKind = elementsSeparatorKind;
}

lpsrParallelMusicBLock::~lpsrParallelMusicBLock ()
{}

void lpsrParallelMusicBLock::acceptIn (basevisitor* v)
{
#ifdef TRACING_IS_ENABLED
  if (gGlobalLpsrOahGroup->getTraceLpsrVisitors ()) {
    gLogStream <<
      "% ==> lpsrParallelMusicBLock::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrParallelMusicBLock>*
    p =
      dynamic_cast<visitor<S_lpsrParallelMusicBLock>*> (v)) {
        S_lpsrParallelMusicBLock elem = this;

#ifdef TRACING_IS_ENABLED
        if (gGlobalLpsrOahGroup->getTraceLpsrVisitors ()) {
          gLogStream <<
            "% ==> Launching lpsrParallelMusicBLock::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void lpsrParallelMusicBLock::acceptOut (basevisitor* v)
{
#ifdef TRACING_IS_ENABLED
  if (gGlobalLpsrOahGroup->getTraceLpsrVisitors ()) {
    gLogStream <<
      "% ==> lpsrParallelMusicBLock::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrParallelMusicBLock>*
    p =
      dynamic_cast<visitor<S_lpsrParallelMusicBLock>*> (v)) {
        S_lpsrParallelMusicBLock elem = this;

#ifdef TRACING_IS_ENABLED
        if (gGlobalLpsrOahGroup->getTraceLpsrVisitors ()) {
          gLogStream <<
            "% ==> Launching lpsrParallelMusicBLock::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void lpsrParallelMusicBLock::browseData (basevisitor* v)
{
#ifdef TRACING_IS_ENABLED
  if (gGlobalLpsrOahGroup->getTraceLpsrVisitors ()) {
    gLogStream <<
      "% ==> lpsrParallelMusicBLock::browseData ()" <<
      endl;
  }
#endif

  for (
    list<S_lpsrPartGroupBlock>::const_iterator i =
      fParallelMusicBLockPartGroupBlocks.begin ();
    i != fParallelMusicBLockPartGroupBlocks.end ();
    ++i
  ) {
    // browse the element browser
    msrBrowser<msrElement> browser (v);
    browser.browse (*(*i));
  } // for

#ifdef TRACING_IS_ENABLED
  if (gGlobalLpsrOahGroup->getTraceLpsrVisitors ()) {
    gLogStream <<
      "% <== lpsrParallelMusicBLock::browseData ()" <<
      endl;
  }
#endif
}

void lpsrParallelMusicBLock::print (ostream& os) const
{
  os <<
    "ParallelMusicBLock" <<
    ", " <<
    singularOrPlural (
      fParallelMusicBLockPartGroupBlocks.size (), "part group", "part groups") <<
    endl;

  ++gIndenter;

  for (
    list<S_lpsrPartGroupBlock>::const_iterator i =
      fParallelMusicBLockPartGroupBlocks.begin ();
    i != fParallelMusicBLockPartGroupBlocks.end ();
    ++i
  ) {
    os << (*i);
  } // for

  --gIndenter;
}

ostream& operator<< (ostream& os, const S_lpsrParallelMusicBLock& elt)
{
  elt->print (os);
  return os;
}


}
