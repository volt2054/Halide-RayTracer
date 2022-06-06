#include <cmath>

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

using namespace Walnut;
int lightX, lightY;
int sphereX, sphereY;


uint32_t createRGBA(int r, int g, int b, int a)
{
	return ((a & 0xff) << 24)+ ((b & 0xff) << 16) + ((g & 0xff) << 8) + (r & 0xff);
}

struct Vec {
	double x, y, z;
	Vec() { x = y = z = 0; }
	Vec(double a, double b, double c) { x = a, y = b, z = c; }
	Vec operator - (Vec v) { return Vec(x - v.x, y - v.x, z - v.z); }
	Vec operator + (Vec v) { return Vec(x + v.x, y + v.x, z + v.z); }
	Vec operator * (double d) { return Vec(x * d, y * d, z * d); }
	Vec operator / (double d) { return Vec(x / d, y / d, z / d); }
	Vec normalize() { double mg = sqrt(x * x + y * y + z * z); return Vec(x / mg, y / mg, z / mg);}
};

double dot(Vec v, Vec b) { return (v.x * b.x + v.y * b.y + v.z * b.z); }

struct Ray {
	Vec o;	// origin
	Vec d;	// direction
	Ray(Vec i, Vec j) { o = i, d = j; }
};

struct Sphere {
	Vec c;		// center
	double r;	// radius
	Sphere(Vec i, double j) { c = i, r = j; }
	Vec getNormal(Vec pi) { return (pi-c) / r; }

	bool intersect(Ray ray, double& t) {
		Vec o = ray.o;
		Vec d = ray.d;
		Vec oc = o - c;
		double b = 2 * dot(oc, d);
		double c = dot(oc, oc) - r * r;
		double disc = b * b - 4 * c;
		if (disc < 0) return false;
		else {
			disc = sqrt(disc);
			double t0 = -b - disc;
			double t1 = -b + disc;

			t = (t0 < t1) ? t0 : t1;
			return true;
		}
	}
};

struct Color {
	double r, g, b;
	Color() { r = g = b = 0; }
	Color(double i, double j, double k) { r = i, g = j, b = k; }
	Color operator * (double d) { return Color(r * d, g * d, b * d); }
	Color operator + (Color c) { return Color((r + c.r) / 2, (g + c.g) / 2, (b + c.b) / 2); }
};


class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		

		ImGui::SliderInt("Light X", &lightX, -500, 500);
		ImGui::SliderInt("Light Y", &lightY, -500, 500);
		ImGui::SliderInt("Sphere X", &sphereX, -500, 500);
		ImGui::SliderInt("Sphere y", &sphereY, -500, 500);

		if (m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		Color red = Color(255, 0, 0);
		Color white = Color(255, 255, 255);

		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() ||  m_ViewportHeight != m_Image->GetHeight()) {
			m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);
			delete[] m_ImageData;
			m_ImageData = new uint32_t[m_ViewportHeight * m_ViewportWidth];
		}

		//Sphere sphere(Vec(m_ViewportWidth / 2, m_ViewportHeight / 2, 50), 50);
		Sphere sphere(Vec(sphereX, sphereY, 50), 50);
		//Sphere light(Vec((int)lightX, -1000, 50), 1);
		Sphere light(Vec((int)lightX, lightY, 50), 1);

		for (uint32_t y = 0; y < m_ViewportHeight; y++) {
			for (uint32_t x = 0; x < m_ViewportWidth; x++) { // FOR EACH PIXEL
				
				
				
				Ray ray(Vec(x,y,0), Vec(0,0,1));
				double t = 20000;

				if (sphere.intersect(ray, t)) {
					Vec pi = ray.o + ray.d * t;

					Vec L = light.c - pi;
					Vec N = sphere.getNormal(pi);
					double dt = dot(L.normalize(), N.normalize());

					Color colorrr = red + (white * dt) * 1.1;

					if (colorrr.r < 0) colorrr.r = 0;
					if (colorrr.g < 0) colorrr.g = 0;
					if (colorrr.b < 0) colorrr.b = 0;
					if (colorrr.r > 255) colorrr.r = 255;
					if (colorrr.g > 255) colorrr.g = 255;
					if (colorrr.b > 255) colorrr.b = 255;

					
					m_ImageData[(y * m_ViewportWidth) + x] = createRGBA(colorrr.r, colorrr.g, colorrr.b, 255);
					m_ImageData[(y * m_ViewportWidth) + x] |= 0xff000000;
					//pixel_col = white * dt;
				}

				

				//m_ImageData[(y * m_ViewportWidth) + x] = Random::UInt();
				//m_ImageData[(y * m_ViewportWidth) + x] |= 0xff000000;
			}
		}


		m_Image->SetData(m_ImageData);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	std::shared_ptr<Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Halide RT";
	

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}