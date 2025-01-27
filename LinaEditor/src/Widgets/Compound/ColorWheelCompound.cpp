/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Editor/Widgets/Compound/ColorWheelCompound.hpp"
#include "Editor/EditorLocale.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Widgets/FX/ColorWheel.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/GUI/Widgets/Layout/Popup.hpp"
#include "Core/GUI/Widgets/Primitives/ColorSlider.hpp"
#include "Core/GUI/Widgets/Primitives/Dropdown.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/InputField.hpp"
#include "Core/GUI/Widgets/Primitives/ColorField.hpp"
#include "Core/GUI/Widgets/Layout/DirectionalLayout.hpp"
#include "Core/Graphics/CommonGraphics.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"
#include "Core/Application.hpp"
#include "Common/Math/Math.hpp"
#include "Common/Platform/LinaVGIncl.hpp"

namespace Lina::Editor
{
	ColorWheelCompound::ColorComponent ColorWheelCompound::ConstructColorComponent(const String& label, float* val)
	{
		const float baseItemHeight = Theme::GetDef().baseItemHeight;

		Text* text			  = m_manager->Allocate<Text>("ColorComponentText");
		text->GetProps().text = label;
		text->GetFlags().Set(WF_POS_ALIGN_Y);
		text->SetAlignedPosY(0.5f);
		text->SetAnchorY(Anchor::Center);

		InputField* field				= m_manager->Allocate<InputField>("ColorComponentInputField");
		field->GetProps().isNumberField = true;
		field->GetProps().valueMin		= 0.0f;
		field->GetProps().valueMax		= 1.0f;
		field->GetProps().valueStep		= 0.01f;
		field->GetProps().valuePtr		= reinterpret_cast<uint8*>(val);
		field->GetProps().clampNumber	= true;
		field->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_USE_FIXED_SIZE_X | WF_POS_ALIGN_Y);
		field->SetAlignedSizeY(1.0f);
		field->SetFixedSizeX(baseItemHeight * 2.0f);
		field->SetAlignedPosY(0.0f);
		field->GetCallbacks().onEdited = [this]() { Recalculate(true, true); };

		ColorSlider* slider						= m_manager->Allocate<ColorSlider>("ColorComponentColorSlider");
		slider->GetWidgetProps().drawBackground = true;
		slider->GetProps().minValue				= 0.0f;
		slider->GetProps().maxValue				= 1.0f;
		slider->GetProps().value				= val;
		slider->GetProps().step					= 0.0f;
		slider->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_SIZE_ALIGN_X | WF_POS_ALIGN_Y);
		slider->SetAlignedSize(Vector2(0.0f, 1.0f));
		slider->SetAlignedPosY(0.0f);
		slider->GetCallbacks().onEdited = [this]() { Recalculate(true, true); };

		DirectionalLayout* layout			  = m_manager->Allocate<DirectionalLayout>("ColorComponentRow");
		layout->GetWidgetProps().childPadding = Theme::GetDef().baseIndent;
		layout->GetFlags().Set(WF_SIZE_ALIGN_X | WF_POS_ALIGN_X | WF_USE_FIXED_SIZE_Y);
		layout->SetAlignedSizeX(1.0f);
		layout->SetAlignedPosX(0.0f);
		layout->SetFixedSizeY(baseItemHeight);
		layout->AddChild(text, field, slider);

		ColorWheelCompound::ColorComponent comp = {
			.row	= layout,
			.field	= field,
			.slider = slider,
		};

		return comp;
	}

	ColorWheelCompound::SaturationValueComponent ColorWheelCompound::ConstructHSVComponent(const String& label, bool isHue, float* val)
	{
		Text* text			  = m_manager->Allocate<Text>("SaturationValueLabel");
		text->GetProps().text = label;
		text->GetFlags().Set(WF_POS_ALIGN_X);
		text->SetAlignedPosX(0.5f);
		text->SetAnchorX(Anchor::Center);

		InputField* field				= m_manager->Allocate<InputField>("SaturationValueField");
		field->GetProps().isNumberField = true;
		field->GetProps().clampNumber	= true;
		field->GetProps().valueMin		= 0.0f;
		field->GetProps().valueMax		= isHue ? 360.0f : 1.0f;
		field->GetProps().valueStep		= isHue ? 1.0f : 0.01f;
		field->GetProps().valuePtr		= reinterpret_cast<uint8*>(val);
		field->GetProps().decimals		= isHue ? 0 : 3;
		field->GetFlags().Set(WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y | WF_POS_ALIGN_X);
		field->SetAlignedSizeX(1.0f);
		field->SetAlignedPosX(0.0f);
		field->SetFixedSizeY(Theme::GetDef().baseItemHeight);
		field->GetCallbacks().onEdited = [this]() { Recalculate(false, true); };

		ColorSlider* slider								  = m_manager->Allocate<ColorSlider>("HSVSlider");
		slider->GetWidgetProps().colorBackgroundDirection = DirectionOrientation::Vertical;
		slider->GetFlags().Set(WF_SIZE_ALIGN_X | WF_POS_ALIGN_X | WF_SIZE_ALIGN_Y);
		slider->SetAlignedSizeX(0.5f);
		slider->SetAlignedPosX(0.5f);
		slider->SetAnchorX(Anchor::Center);
		slider->SetAlignedSizeY(0.0f);
		slider->GetProps().minValue				= 0.0f;
		slider->GetProps().maxValue				= isHue ? 360.0f : 1.0f;
		slider->GetProps().value				= val;
		slider->GetProps().step					= 0.0f;
		slider->GetWidgetProps().lvgUserData	= isHue ? &m_colorSliderUserData : nullptr;
		slider->GetCallbacks().onEdited			= [this]() { Recalculate(false, true); };
		slider->GetWidgetProps().drawBackground = true;

		// Layout
		DirectionalLayout* layout			  = m_manager->Allocate<DirectionalLayout>("SaturationValueLayout");
		layout->GetProps().direction		  = DirectionOrientation::Vertical;
		layout->GetWidgetProps().childPadding = Theme::GetDef().baseIndent;
		layout->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_POS_ALIGN_Y | WF_USE_FIXED_SIZE_X);
		layout->SetFixedSizeX(Theme::GetDef().baseItemHeight * 2.0f);
		layout->SetAlignedPosY(0.0f);
		layout->SetAlignedSizeY(1.0f);
		layout->AddChild(slider, field, text);

		SaturationValueComponent comp = {
			.layout = layout,
			.slider = slider,
			.field	= field,
			.text	= text,
		};

		return comp;
	}

	void ColorWheelCompound::Construct()
	{
		m_colorSliderUserData.specialType = GUISpecialType::VerticalHue;
		m_editor						  = Editor::Get();
		const float		   baseItemHeight = Theme::GetDef().baseItemHeight;
		DirectionalLayout* vertical		  = m_manager->Allocate<DirectionalLayout>();
		vertical->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		vertical->SetAlignedPos(Vector2::Zero);
		vertical->SetAlignedSize(Vector2::One);
		vertical->GetProps().direction = DirectionOrientation::Vertical;
		AddChild(vertical);

		// Top row
		m_topRow = m_manager->Allocate<DirectionalLayout>("TopRow");
		m_topRow->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		m_topRow->SetAlignedPosX(0.0f);
		m_topRow->SetAlignedSize(Vector2(1.0f, 0.4f));
		m_topRow->GetProps().direction					  = DirectionOrientation::Horizontal;
		m_topRow->GetWidgetProps().borderThickness.bottom = Theme::GetDef().baseOutlineThickness * 2.0f;
		m_topRow->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
		m_topRow->SetAlignedPos(Vector2::Zero);
		vertical->AddChild(m_topRow);

		Widget* topRowLeftSide = m_manager->Allocate<Widget>("TopRowLeftSide");
		topRowLeftSide->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y | WF_SIZE_X_COPY_Y);
		topRowLeftSide->SetAlignedPosY(0.0f);
		topRowLeftSide->SetAlignedSize(Vector2::One);
		topRowLeftSide->GetWidgetProps().childMargins = TBLR::Eq(Theme::GetDef().baseIndent);
		m_topRow->AddChild(topRowLeftSide);

		// Wheel.
		m_wheel = m_manager->Allocate<ColorWheel>("Wheel");
		m_wheel->GetFlags().Set(WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y | WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
		m_wheel->SetAlignedSize(Vector2::One);
		m_wheel->SetAlignedPos(Vector2::Zero);
		m_wheel->GetProps().hue			 = &m_hsv.x;
		m_wheel->GetProps().saturation	 = &m_hsv.y;
		m_wheel->GetCallbacks().onEdited = [this]() { Recalculate(false, true); };
		topRowLeftSide->AddChild(m_wheel);

		// HSV
		DirectionalLayout* hsvRow					  = m_manager->Allocate<DirectionalLayout>("HSVRow");
		hsvRow->GetWidgetProps().borderThickness.left = Theme::GetDef().baseOutlineThickness;
		hsvRow->GetWidgetProps().childMargins		  = TBLR::Eq(Theme::GetDef().baseIndent);
		hsvRow->GetFlags().Set(WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y | WF_POS_ALIGN_Y);
		hsvRow->SetAlignedSize(Vector2(0.0f, 1.0f));
		hsvRow->SetAlignedPosY(0.0f);
		m_topRow->AddChild(hsvRow);

		// Hue
		m_hueComponent = ConstructHSVComponent("H", true, &m_hsv.x);
		m_hueComponent.layout->GetFlags().Set(WF_POS_ALIGN_X);
		m_hueComponent.layout->SetAlignedPosX(0.0f);
		m_hueComponent.layout->SetAnchorX(Anchor::Start);
		hsvRow->AddChild(m_hueComponent.layout);

		// Sat
		m_saturationComponent = ConstructHSVComponent("S", false, &m_hsv.y);
		m_saturationComponent.layout->GetFlags().Set(WF_POS_ALIGN_X);
		m_saturationComponent.layout->SetAlignedPosX(0.5f);
		m_saturationComponent.layout->SetAnchorX(Anchor::Center);
		hsvRow->AddChild(m_saturationComponent.layout);

		// Val
		m_valueComponent = ConstructHSVComponent("V", false, &m_hsv.z);
		m_valueComponent.layout->GetFlags().Set(WF_POS_ALIGN_X);
		m_valueComponent.layout->SetAlignedPosX(1.0f);
		m_valueComponent.layout->SetAnchorX(Anchor::End);
		hsvRow->AddChild(m_valueComponent.layout);

		// Bottom Col
		m_bottomColumn = m_manager->Allocate<DirectionalLayout>("BottomRow");
		m_bottomColumn->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		m_bottomColumn->SetAlignedPosX(0.0f);
		m_bottomColumn->SetAlignedSize(Vector2(1.0f, 0.0f));
		m_bottomColumn->GetProps().direction		  = DirectionOrientation::Vertical;
		m_bottomColumn->GetProps().mode				  = DirectionalLayout::Mode::EqualPositions;
		m_bottomColumn->GetWidgetProps().childMargins = TBLR::Eq(Theme::GetDef().baseIndent);
		m_bottomColumn->GetWidgetProps().childPadding = Theme::GetDef().baseIndent;
		vertical->AddChild(m_bottomColumn);

		// Hex and old/new color fields
		DirectionalLayout* hexAndColorsRow = m_manager->Allocate<DirectionalLayout>("HexAndColorsRow");
		hexAndColorsRow->GetFlags().Set(WF_USE_FIXED_SIZE_Y | WF_POS_ALIGN_X | WF_SIZE_ALIGN_X);
		hexAndColorsRow->SetAlignedPosX(0.0f);
		hexAndColorsRow->SetFixedSizeY(baseItemHeight);
		hexAndColorsRow->SetAlignedSizeX(1.0f);
		m_bottomColumn->AddChild(hexAndColorsRow);

		// Hex
		m_hexField							   = m_manager->Allocate<InputField>("HexField");
		m_hexField->GetCallbacks().onEditEnded = [this]() {
			const String& str = m_hexField->GetText()->GetProps().text;
			m_editedColor.FromHex(str);
			m_editedColor255 = m_editedColor * 255.0f;
			m_editedColor255.Round();
			Recalculate(true, true);
			PropagateCBOnEditEnded();
		};
		m_hexField->GetFlags().Set(WF_USE_FIXED_SIZE_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y);
		m_hexField->SetFixedSizeX(baseItemHeight * 6);
		m_hexField->SetAlignedSizeY(1.0f);
		m_hexField->SetAlignedPosY(0.0f);
		hexAndColorsRow->AddChild(m_hexField);

		// Right side
		DirectionalLayout* oldAndNewColors = m_manager->Allocate<DirectionalLayout>("OldAndNewColors");
		oldAndNewColors->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_USE_FIXED_SIZE_X | WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
		oldAndNewColors->SetAlignedSizeY(1.0f);
		oldAndNewColors->SetFixedSizeX(baseItemHeight * 6.0f);
		oldAndNewColors->SetAlignedPos(Vector2(1.0f, 0.0f));
		oldAndNewColors->SetAnchorX(Anchor::End);
		oldAndNewColors->GetProps().mode = DirectionalLayout::Mode::EqualSizes;
		hexAndColorsRow->AddChild(oldAndNewColors);

		// Old color
		m_oldColorField					  = m_manager->Allocate<ColorField>("OldColor");
		m_oldColorField->GetProps().value = &m_oldColor;
		m_oldColorField->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_POS_ALIGN_Y);
		m_oldColorField->SetAlignedPosY(0.0f);
		m_oldColorField->SetAlignedSizeY(1.0f);
		m_oldColorField->GetProps().backgroundTexture	   = m_editor->GetApp()->GetResourceManager().GetResource<Texture>(EDITOR_TEXTURE_CHECKERED_ID);
		m_oldColorField->GetProps().convertToLinear		   = true;
		m_oldColorField->GetWidgetProps().outlineThickness = 0.0f;
		m_oldColorField->GetWidgetProps().rounding		   = 0.0f;
		oldAndNewColors->AddChild(m_oldColorField);

		// New color
		m_newColorField					  = m_manager->Allocate<ColorField>("NewColor");
		m_newColorField->GetProps().value = &m_editedColor;
		m_newColorField->GetFlags().Set(WF_SIZE_ALIGN_Y | WF_POS_ALIGN_Y);
		m_newColorField->SetAlignedPosY(0.0f);
		m_newColorField->SetAlignedSizeY(1.0f);
		m_newColorField->GetProps().backgroundTexture	   = m_editor->GetApp()->GetResourceManager().GetResource<Texture>(EDITOR_TEXTURE_CHECKERED_ID);
		m_newColorField->GetProps().convertToLinear		   = true;
		m_newColorField->GetWidgetProps().outlineThickness = 0.0f;
		m_newColorField->GetWidgetProps().rounding		   = 0.0f;
		oldAndNewColors->AddChild(m_newColorField);

		DirectionalLayout* displayAndThemeRow = m_manager->Allocate<DirectionalLayout>("DisplayAndThemeRow");
		displayAndThemeRow->GetFlags().Set(WF_USE_FIXED_SIZE_Y | WF_POS_ALIGN_X | WF_SIZE_ALIGN_X);
		displayAndThemeRow->SetAlignedPosX(0.0f);
		displayAndThemeRow->SetFixedSizeY(baseItemHeight);
		displayAndThemeRow->SetAlignedSizeX(1.0f);
		m_bottomColumn->AddChild(displayAndThemeRow);

		// Display dropdown
		Dropdown* displayDropdown = m_manager->Allocate<Dropdown>("ColorDisplayDropdown");
		displayDropdown->GetFlags().Set(WF_USE_FIXED_SIZE_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y);
		displayDropdown->SetFixedSizeX(baseItemHeight * 6);
		displayDropdown->SetAlignedSizeY(1.0f);
		displayDropdown->SetAlignedPosY(0.0f);

		static HashMap<ColorDisplay, String> COLOR_DISPLAY_VALUES = {
			{ColorDisplay::RGB, "RGB (0-1)"},
			{ColorDisplay::RGB255, "RGB (0-255)"},
		};

		displayDropdown->GetProps().onSelected = [this](int32 item, String& outNewTitle) -> bool {
			SwitchColorDisplay(static_cast<ColorDisplay>(item));
			outNewTitle = COLOR_DISPLAY_VALUES[static_cast<ColorDisplay>(item)];
			return true;
		};
		displayDropdown->GetProps().onAddItems = [this](Popup* popup) {
			for (int32 i = 0; i < static_cast<int32>(ColorDisplay::MAX); i++)
				popup->AddToggleItem(COLOR_DISPLAY_VALUES[static_cast<ColorDisplay>(i)], i == static_cast<int32>(m_selectedDisplay), i);
		};

		displayDropdown->GetText()->GetProps().text = COLOR_DISPLAY_VALUES[m_selectedDisplay];
		displayDropdown->Initialize();
		displayAndThemeRow->AddChild(displayDropdown);

		const Vector<String> items = {"Background0",	"Background1",	  "Background2",	"Background3",	   "Background4", "Background5", "Foreground0",		 "Foreground1",			 "Silent0", "Silent1", "Silent2", "AccentPrimary0",
									  "AccentPrimary1", "AccentPrimary2", "AccentPrimary3", "AccentSecondary", "AccentError", "AccentWarn",	 "OutlineColorBase", "OutlineColorControls", "Black"};

		Dropdown* themeDropdown = m_manager->Allocate<Dropdown>("ThemeDropdown");
		themeDropdown->GetFlags().Set(WF_USE_FIXED_SIZE_X | WF_POS_ALIGN_Y | WF_POS_ALIGN_X | WF_SIZE_ALIGN_Y);
		themeDropdown->SetFixedSizeX(baseItemHeight * 6);
		themeDropdown->SetAlignedSizeY(1.0f);
		themeDropdown->SetAlignedPosY(0.0f);
		themeDropdown->SetAnchorX(Anchor::End);
		themeDropdown->SetAlignedPosX(1.0f);
		themeDropdown->GetText()->GetProps().text = Locale::GetStr(LocaleStr::ThemeColor);
		themeDropdown->GetProps().onSelected	  = [this, items](int32 item, String& outNewTitle) -> bool {
			 const linatl::array<Color, 21> themeColors = {
				 Theme::GetDef().background0,	 Theme::GetDef().background1,	  Theme::GetDef().background2, Theme::GetDef().background3, Theme::GetDef().background4,	  Theme::GetDef().background5,			Theme::GetDef().foreground0,
				 Theme::GetDef().foreground1,	 Theme::GetDef().silent0,		  Theme::GetDef().silent1,	   Theme::GetDef().silent2,		Theme::GetDef().accentPrimary0,	  Theme::GetDef().accentPrimary1,		Theme::GetDef().accentPrimary2,
				 Theme::GetDef().accentPrimary3, Theme::GetDef().accentSecondary, Theme::GetDef().accentError, Theme::GetDef().accentWarn,	Theme::GetDef().outlineColorBase, Theme::GetDef().outlineColorControls, Theme::GetDef().black,
			 };

			 outNewTitle	   = items[item];
			 const Color color = themeColors[item];
			 SetTargetColor(color);
			 PropagateCBOnEdited();
			 return true;
		};
		themeDropdown->GetProps().onAddItems = [this, items](Popup* popup) {
			int32 idx = 0;
			for (const String& item : items)
				popup->AddToggleItem(item, false, idx++);
		};
		displayAndThemeRow->AddChild(themeDropdown);

		// Color display
		m_colorComp1									  = ConstructColorComponent("R", &m_editedColor.x);
		m_colorComp2									  = ConstructColorComponent("G", &m_editedColor.y);
		m_colorComp3									  = ConstructColorComponent("B", &m_editedColor.z);
		m_colorComp4									  = ConstructColorComponent("A", &m_editedColor.w);
		m_colorComp4.slider->GetProps().backgroundTexture = m_editor->GetApp()->GetResourceManager().GetResource<Texture>(EDITOR_TEXTURE_CHECKERED_ID);
		m_bottomColumn->AddChild(m_colorComp1.row, m_colorComp2.row, m_colorComp3.row, m_colorComp4.row);
	}

	void ColorWheelCompound::Initialize()
	{
		Widget::Initialize();
		Recalculate(true, true);
	}

	void ColorWheelCompound::SetTargetColor(const Color& col, bool callback)
	{
		m_editedColor = col.Linear2SRGB();
		Recalculate(true, callback);
	}

	void ColorWheelCompound::FetchFromTrackedColors()
	{
		if (m_props.trackColorv3)
		{
			m_editedColor = Color(m_props.trackColorv3->x, m_props.trackColorv3->y, m_props.trackColorv3->z, 1.0f).Linear2SRGB();
			if (m_editedColor.x < 0.0f)
				m_editedColor.x = 0.0f;
			if (m_editedColor.y < 0.0f)
				m_editedColor.y = 0.0f;
			if (m_editedColor.z < 0.0f)
				m_editedColor.z = 0.0f;
			Recalculate(true, false);
		}
		else if (m_props.trackColorv4)
		{
			m_editedColor = Color(m_props.trackColorv4->x, m_props.trackColorv4->y, m_props.trackColorv4->z, m_props.trackColorv4->w).Linear2SRGB();
			if (m_editedColor.x < 0.0f)
				m_editedColor.x = 0.0f;
			if (m_editedColor.y < 0.0f)
				m_editedColor.y = 0.0f;
			if (m_editedColor.z < 0.0f)
				m_editedColor.z = 0.0f;
			if (m_editedColor.w < 0.0f)
				m_editedColor.w = 0.0f;
			Recalculate(true, false);
		}
	}

	void ColorWheelCompound::SwitchColorDisplay(ColorDisplay display)
	{
		m_selectedDisplay = display;

		if (m_selectedDisplay == ColorDisplay::RGB255)
		{
			m_editedColor255 = m_editedColor * 255.0f;
			m_editedColor255.Round();

			m_colorComp1.slider->GetProps().maxValue = 255.0f;
			m_colorComp2.slider->GetProps().maxValue = 255.0f;
			m_colorComp3.slider->GetProps().maxValue = 255.0f;
			m_colorComp4.slider->GetProps().maxValue = 255.0f;
			m_colorComp1.slider->GetProps().value	 = &m_editedColor255.x;
			m_colorComp2.slider->GetProps().value	 = &m_editedColor255.y;
			m_colorComp3.slider->GetProps().value	 = &m_editedColor255.z;
			m_colorComp4.slider->GetProps().value	 = &m_editedColor255.w;

			m_colorComp1.field->GetProps().valueMax = 255.0f;
			m_colorComp2.field->GetProps().valueMax = 255.0f;
			m_colorComp3.field->GetProps().valueMax = 255.0f;
			m_colorComp4.field->GetProps().valueMax = 255.0f;

			m_colorComp1.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor255.x);
			m_colorComp2.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor255.y);
			m_colorComp3.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor255.z);
			m_colorComp4.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor255.w);
			m_colorComp1.field->GetProps().decimals = 0;
			m_colorComp2.field->GetProps().decimals = 0;
			m_colorComp3.field->GetProps().decimals = 0;
			m_colorComp4.field->GetProps().decimals = 0;
		}
		else
		{
			m_editedColor = m_editedColor255 / 255.0f;

			m_colorComp1.slider->GetProps().maxValue = 1.0f;
			m_colorComp2.slider->GetProps().maxValue = 1.0f;
			m_colorComp3.slider->GetProps().maxValue = 1.0f;
			m_colorComp4.slider->GetProps().maxValue = 1.0f;
			m_colorComp1.slider->GetProps().value	 = &m_editedColor.x;
			m_colorComp2.slider->GetProps().value	 = &m_editedColor.y;
			m_colorComp3.slider->GetProps().value	 = &m_editedColor.z;
			m_colorComp4.slider->GetProps().value	 = &m_editedColor.w;

			m_colorComp1.field->GetProps().valueMax = 1.0f;
			m_colorComp2.field->GetProps().valueMax = 1.0f;
			m_colorComp3.field->GetProps().valueMax = 1.0f;
			m_colorComp4.field->GetProps().valueMax = 1.0f;
			m_colorComp1.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor.x);
			m_colorComp2.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor.y);
			m_colorComp3.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor.z);
			m_colorComp4.field->GetProps().valuePtr = reinterpret_cast<uint8*>(&m_editedColor.w);
			m_colorComp1.field->GetProps().decimals = 3;
			m_colorComp2.field->GetProps().decimals = 3;
			m_colorComp3.field->GetProps().decimals = 3;
			m_colorComp4.field->GetProps().decimals = 3;
		}
	}

	void ColorWheelCompound::Recalculate(bool sourceRGB, bool callback)
	{
		if (m_selectedDisplay == ColorDisplay::RGB255)
			m_editedColor = m_editedColor255 / 255.0f;
		else
		{
			m_editedColor255 = m_editedColor * 255.0f;
			m_editedColor255.Round();
		}

		if (sourceRGB)
			m_hsv = m_editedColor.SRGB2HSV();
		else
		{
			m_editedColor	 = m_hsv.HSV2SRGB();
			m_editedColor255 = m_editedColor * 255.0f;
			m_editedColor255.Round();
		}

		const Color begin													 = Color(m_hsv.x, 1.0f, 1.0f).HSV2SRGB();
		m_valueComponent.slider->GetWidgetProps().colorBackground.start		 = Color(m_hsv.x, m_hsv.y, 1.0).HSV2SRGB();
		m_saturationComponent.slider->GetWidgetProps().colorBackground.start = begin.SRGB2Linear();

		m_valueComponent.slider->GetWidgetProps().colorBackground.end	   = Color::Black;
		m_saturationComponent.slider->GetWidgetProps().colorBackground.end = Color::White;

		m_colorComp1.slider->GetWidgetProps().colorBackground.start = Color(0.0f, m_editedColor.y, m_editedColor.z, 1.0f).SRGB2Linear();
		m_colorComp1.slider->GetWidgetProps().colorBackground.end	= Color(1.0f, m_editedColor.y, m_editedColor.z, 1.0f).SRGB2Linear();
		m_colorComp2.slider->GetWidgetProps().colorBackground.start = Color(m_editedColor.x, 0.0f, m_editedColor.z, 1.0f).SRGB2Linear();
		m_colorComp2.slider->GetWidgetProps().colorBackground.end	= Color(m_editedColor.x, 1.0f, m_editedColor.z, 1.0f).SRGB2Linear();
		m_colorComp3.slider->GetWidgetProps().colorBackground.start = Color(m_editedColor.x, m_editedColor.y, 0.0f, 1.0f).SRGB2Linear();
		m_colorComp3.slider->GetWidgetProps().colorBackground.end	= Color(m_editedColor.x, m_editedColor.y, 1.0f, 1.0f).SRGB2Linear();
		m_colorComp4.slider->GetWidgetProps().colorBackground.start = Color(m_editedColor.x, m_editedColor.y, m_editedColor.z, 0.0f).SRGB2Linear();
		m_colorComp4.slider->GetWidgetProps().colorBackground.end	= Color(m_editedColor.x, m_editedColor.y, m_editedColor.z, 1.0f).SRGB2Linear();
		m_wheel->GetProps().darknessAlpha							= m_hsv.z;

		if (callback && m_props.trackColorv3)
		{
			const Color col		  = m_editedColor.SRGB2Linear();
			*m_props.trackColorv3 = Vector3(col.x, col.y, col.z);
		}

		if (callback && m_props.trackColorv4)
		{
			const Color col		  = m_editedColor.SRGB2Linear();
			*m_props.trackColorv4 = Vector4(col.x, col.y, col.z, col.w);
		}

		if (callback)
			PropagateCBOnEdited();

		m_hexField->GetText()->GetProps().text = m_editedColor.GetHex();
		m_hexField->GetText()->CalculateTextSize();
	}

} // namespace Lina::Editor
