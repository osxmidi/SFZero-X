#include "ClickableLabel.h"

sfzero::ClickableLabel::ClickableLabel(const String &componentName, const String &labelText) : Label(componentName, labelText) {}
void sfzero::ClickableLabel::addClickListener(ClickListener *listener) { clickListeners.add(listener); }
void sfzero::ClickableLabel::removeClickListener(ClickListener *listener) { clickListeners.remove(listener); }
void sfzero::ClickableLabel::mouseUp(const MouseEvent &e)
{
  bool goodClick = e.mouseWasClicked() && contains(e.getPosition()) && !e.mods.isPopupMenu();

  if (goodClick)
  {
    BailOutChecker checker(this);
    clickListeners.callChecked(checker, &ClickListener::labelClicked, this);
  }
}
