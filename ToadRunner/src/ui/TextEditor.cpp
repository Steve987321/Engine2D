#include "pch.h"

#include "engine/Engine.h"
#include "TextEditor.h"

namespace Toad {

    TextEditor::TextEditor() = default;

    TextEditor::~TextEditor() = default;

    void TextEditor::Show(std::string &txt) {
        ImGui::InputTextMultiline("TextEdit", txt.data(), sizeof txt.data());
    }

} // Toad