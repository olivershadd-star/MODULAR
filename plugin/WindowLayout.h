#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace modular_ui {
// Display areas are JUCE logical coordinates, already adjusted for Windows DPI.
inline juce::Point<int> initialEditorSize(juce::Rectangle<int> work){
 return {juce::jlimit(1,960,work.getWidth()-64),juce::jlimit(1,640,work.getHeight()-112)};
}
inline juce::Rectangle<int> fittedWindow(juce::Rectangle<int> bounds,juce::Rectangle<int> work){
 auto safe=work.reduced(12);if(safe.isEmpty())safe=work;
 int width=juce::jlimit(1,safe.getWidth(),bounds.getWidth()),height=juce::jlimit(1,safe.getHeight(),bounds.getHeight());
 return {juce::jlimit(safe.getX(),safe.getRight()-width,bounds.getX()),juce::jlimit(safe.getY(),safe.getBottom()-height,bounds.getY()),width,height};
}
inline void enableWindowControls(juce::DocumentWindow&window){
 window.setTitleBarButtonsRequired(juce::DocumentWindow::allButtons,false);
}
}
