namespace Toad
{

// centering a single line of text inside a box on the X axis
template<typename ...Args>
void center_textX(const ImVec4& col = {1,1,1,1}, const char* frmt = "sample text", Args&& ...args)
{	
	ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2.f - ImGui::CalcTextSize(frmt).x / 2.f);
	ImGui::TextColored(col, frmt, args...);
}

// centering a single line of text inside a box on the X and Y axis
template<typename ...Args>
void center_text(const ImVec4& col = {1,1,1,1}, const char* frmt = "sample text", Args&& ...args)
{
	ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2.f - ImGui::CalcTextSize(frmt).x / 2.f);
	ImGui::SetCursorPosY(ImGui::GetWindowSize().y / 2.f - ImGui::CalcTextSize(frmt).y / 2.f);
	ImGui::TextColored(col, frmt, args...);
}

}