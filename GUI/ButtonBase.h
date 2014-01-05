#ifndef _BUTTONBASE_
#define _BUTTONBASE_

#include "IElement.h"
#include "IInput.h"
#include "IRenderer.h"
#include "VecMath.h"
#include <glm/glm.hpp>
#include <string>
#include <functional>

namespace UI
{
	template< class T >
	class GUIFactory;

	class Button : public IElement
	{
	public:

		typedef std::function<void(void)> DELEGATE;

		void Update(IInput&, double) override;
		void Render(IRenderer&) override;

		void Select() override;
		void Deselect() override;
		void Trigger() override;

		void SetCallback(const DELEGATE& callback);

		void SetText(std::string str);

	protected:

		Button(const Math::FRECT& s, const glm::vec3& defaultColor,
			const glm::vec3& hoverColor, float scale, const std::string& str, const DELEGATE& callback);

		virtual ~Button() {}

		Math::FRECT m_sprite;

	private:

		glm::vec3 m_defaultColor;
		glm::vec3 m_hoverColor;
		float m_scale;

		std::string m_text;

		DELEGATE m_callback;

		bool m_bMouseHover;
		bool m_bSelected;

		friend class GUIFactory<Button>;
	};

}

#endif // _BUTTONBASE_