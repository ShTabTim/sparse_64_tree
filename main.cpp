#include <windows.h>
#include <gl/GL.h>
#include <vector>
#include "highgl/highgl.hpp"
#include <string>
#include <unordered_map>

#include "shaders_source.hpp"
#include <chrono>

#define USE_STANDART_MATH
#include "basic_math_includes.hpp"
#include "vec3.hpp"
#include "mat3.hpp"
#include "quat.hpp"

#define ERROR_ERROR_NAME L"Ошибка"
#define error_named(e, n) MessageBoxW(NULL, e, n, MB_ICONERROR|MB_OK)
#define error(e) MessageBoxW(NULL, e, ERROR_ERROR_NAME, MB_ICONERROR|MB_OK)

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

GLuint compile_shader(GLenum type, const GLchar* source, const std::wstring& name) {
	GLint success;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint log_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0) {
			std::string info_log;
			info_log.resize(log_length - 1);
			glGetShaderInfoLog(shader, log_length, nullptr, info_log.data());
			error_named(std::wstring(info_log.begin(), info_log.end()).c_str(), (L"Ошибка компиляции шейдера " + name).c_str());
			info_log.clear();
		}
		else error((L"Ошибка компиляции шейдера " + name + L", но без лога").c_str());
		glDeleteShader(shader);
	}
	return shader;
}

typedef struct shader_program {
	std::unordered_map<std::string, GLint> uniform_locations;
	GLuint id = 0;
	template<typename... Args>
		requires (std::same_as<Args, GLuint> && ...)
	void init(bool delete_shaders, Args... args) {
		if (id && glIsProgram(id)) {
			GLint numShaders = 0;
			glGetProgramiv(id, GL_ATTACHED_SHADERS, &numShaders);
			if (numShaders > 0) {
				std::vector<GLuint> shaders(numShaders);
				glGetAttachedShaders(id, numShaders, NULL, shaders.data());
				for (GLuint s : shaders) glDetachShader(id, s);
			}
		}
		else id = glCreateProgram();
		(glAttachShader(id, args), ...);

		glLinkProgram(id);
		{
			GLint success;
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			if (!success) {
				GLint log_length;
				glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);
				if (log_length > 0) {
					std::string info_log;
					info_log.resize(log_length - 1);
					glGetProgramInfoLog(id, log_length, nullptr, info_log.data());
					error_named(std::wstring(info_log.begin(), info_log.end()).c_str(), L"Ошибка линка шейдерной программы.");
					info_log.clear();
				}
				else error(L"Ошибка линка шейдерной прогрыммы, но без лога.");
				glDeleteProgram(id);
			}
		}
		if (delete_shaders) ((glIsShader(args) ? glDeleteShader(args) : void()), ...);

		glUseProgram(id);
		uniform_locations.clear();
		{
			GLint uniforms_count = 0;
			glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniforms_count);
			if (uniforms_count <= 0) return;
			GLint max_uniform_lenght = 0;
			glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_lenght);
			std::vector<GLchar> name(max_uniform_lenght + 1);
			for (uint32_t t(uniforms_count); t--;) {
				GLsizei length;
				GLint size;
				GLenum type;
				glGetActiveUniform(id, t, max_uniform_lenght, &length, &size, &type, name.data());
				uniform_locations[name.data()] = glGetUniformLocation(id, name.data());
			}
			name.clear();
		}
	}
	void finit() {
		if (glIsProgram(id)) glDeleteProgram(id);
		uniform_locations.clear();
	}
} shader_program;

#define USE_EMPTY

uint32_t width;
uint32_t height;
uint32_t restore_cursor_x = 0;
uint32_t restore_cursor_y = 0;
bool cursor_disable = true;
bool mouse_in;

float pos_speed = 5;
float ang_speed = 2;
vec3<float> ang = { 0.f, 0.f, 0.f };
vec3<float> pos = { 0.f, 0.f, 0.f };

class node_64_tree {
public:
	uint64_t leaf_mask = 0xFFFFFFFFFFFFFFFFull;
#ifdef USE_EMPTY
	uint64_t empty_mask = 0xFFFFFFFFFFFFFFFFull;
#endif//USE_EMPTY
	uint32_t child[64] = { 0 };
};

class tree_64 {
public:
	std::vector<node_64_tree> big_tree = {};
	uint32_t make_vox() {
		big_tree.emplace_back();
		return big_tree.size() - 1;
	}
	uint32_t make_depth(uint32_t index, uint32_t depth, vec3<uint32_t> p) {
		if (index >= big_tree.size()) return 0;

		uint32_t depth_size = 1 << (2 * depth);
		if ((p.x >= depth_size) || (p.y >= depth_size) || (p.z >= depth_size)) return 0;

		while (depth_size > 1) {
			depth_size >>= 2;
			vec3<uint32_t> local_coord = p / depth_size;
			p -= depth_size * local_coord;
			uint32_t kk = local_coord.x + 4 * (local_coord.y + 4 * local_coord.z);
#ifdef USE_EMPTY
			big_tree[index].empty_mask &= ~(1ull << kk);
#endif//USE_EMPTY
			if (1 & (big_tree[index].leaf_mask >> kk)) {
				big_tree[index].leaf_mask &= ~(1ull << kk);
				big_tree[index].child[kk] = big_tree.size();
				index = big_tree.size();
				big_tree.emplace_back();
			}
			else {
				index = big_tree[index].child[kk];
			}
		}

		return index;
	}
	void place_point(uint32_t index, uint32_t depth, vec3<uint32_t> p, uint32_t color) {
		if(depth >= 1) index = make_depth(index, depth - 1, p / 4);
		p = vec3<uint32_t>(p.x % 4, p.y % 4, p.z % 4);
		big_tree[index].child[p.x + 4 * (p.y + 4 * p.z)] = color;
#ifdef USE_EMPTY
		if (0xFF000000&color) big_tree[index].empty_mask &= ~(1ull << (p.x + 4 * (p.y + 4 * p.z)));
#endif//USE_EMPTY
	}
};

class gl_drawer {
private:
	
	shader_program screen;
	shader_program raytrace;
	uint32_t compute_width = 64, compute_height = 64;
	GLuint ssbo;
	GLuint screen_texture;
	tree_64 tt;
public:
	void init() {
		glGenTextures(1, &screen_texture);
		glBindTexture(GL_TEXTURE_2D, screen_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 16 * compute_width, 16 * compute_height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindImageTexture(0, screen_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		screen.init(true,
			compile_shader(GL_VERTEX_SHADER, shader_source_vert, L"вершинный экрана"),
			compile_shader(GL_FRAGMENT_SHADER, shader_source_frag, L"фрагментный экрана")
		);
		raytrace.init(true,
			compile_shader(GL_COMPUTE_SHADER, shader_source_comp, L"рейтрейсерный")
		);
		uint32_t siz = 4;

		//tt.big_tree.push_back(node_64_tree());
		//
		//for (uint32_t x(siz); x--;)
		//	for (uint32_t z(siz); z--;)
		//		for (uint32_t y((uint32_t)(siz* (0.5f + 0.3f * sinf(x * 3.14159265f / siz) * sinf(z * 3.14159265f / siz) + 0.1f * sinf(x * 2 * 3.14159265f / siz) * sinf(z * 2 * 3.14159265f / siz) + 0.05f * sinf(x * 4 * 3.14159265f / siz) * sinf(z * 4 * 3.14159265f / siz) + 0.05f * sinf(x * 8 * 3.14159265f / siz) * sinf(z * 8 * 3.14159265f / siz)))); y--;) {
		//			tt.place_point(0, 0, vec3<uint32_t>(x, y, z), rand() | ((x / 2) % 0xFF) | ((z / 2) % 0xFF << 8) | 0xFF000000);
		//		}

		tt.big_tree.push_back(node_64_tree());
		tt.big_tree[0].leaf_mask = (0xFFFFFFFFFFFFFFFF & ~1ull) & ~4ull;
		for (uint32_t p(64); p--;) {
			uint32_t r = std::rand() % 2;
			tt.big_tree[0].child[p] = std::rand() % 0xFF << 8;
			tt.big_tree[0].child[p] |= 0xFF000000 * r;
#ifdef USE_EMPTY
			if (r) tt.big_tree[0].empty_mask &= ~(1ull << p);
#endif//USE_EMPTY
		}
		tt.big_tree[0].child[0] |= 0xFF000000;
#ifdef USE_EMPTY
		tt.big_tree[0].empty_mask &= ~1ull;
#endif//USE_EMPTY
		tt.big_tree[0].child[0] = 0;
		tt.big_tree[0].child[2] |= 0xFF000000;
#ifdef USE_EMPTY
		tt.big_tree[0].empty_mask &= ~4ull;
#endif//USE_EMPTY
		tt.big_tree[0].child[2] = 1;
		
		tt.big_tree.push_back(node_64_tree());
		tt.big_tree[1].leaf_mask = 0xFFFFFFFFFFFFFFFF & ~2ull;
		for (uint32_t p(64); p--;) {
			uint32_t r = std::rand() % 2;
			tt.big_tree[1].child[p] = std::rand() % 0xFF << 16;
			tt.big_tree[1].child[p] |= 0xFF000000 * r;
#ifdef USE_EMPTY
			if (r) tt.big_tree[1].empty_mask &= ~(1ull << p);
#endif//USE_EMPTY
		}
		tt.big_tree[1].child[1] |= 0xFF000000;
#ifdef USE_EMPTY
		tt.big_tree[1].empty_mask &= ~2ull;
#endif//USE_EMPTY
		tt.big_tree[1].child[1] = 0;

		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, tt.big_tree.size() * sizeof(node_64_tree), tt.big_tree.data(), GL_STREAM_DRAW);
		glUseProgram(raytrace.id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	}

	void finit() {
		raytrace.finit();
		screen.finit();
		if (glIsTexture(screen_texture)) glDeleteTextures(1, &screen_texture);
		if (glIsBuffer(ssbo)) glDeleteBuffers(1, &ssbo);
	}

	void draw(HDC hdc) {

		static float time = 0;
		time += 0.005f;
		//glClearColor(0.0f, 0.1f+0.1f*sinf(time), 0.0f, 1.0f);

		glUseProgram(raytrace.id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		//std::vector<uint32_t> tt.big_tree = { 0xFFFFFFFF, 0xFFFFFFFF, (uint32_t)std::rand() % 256};
		//glBufferData(GL_SHADER_STORAGE_BUFFER, tt.big_tree.size() * sizeof(float), tt.big_tree.data(), GL_STREAM_DRAW);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
		quat<float>l_ang =
			rot(vec3<float>(1.0f, 0.0f, 0.0f), ang.x) *
			rot(vec3<float>(0.0f, 1.0f, 0.0f), ang.y) *
			rot(vec3<float>(0.0f, 0.0f, 1.0f), ang.z);
		glUniform4f(raytrace.uniform_locations["ang"], l_ang.v.x, l_ang.v.y, l_ang.v.z, l_ang.r);
		glUniform3f(raytrace.uniform_locations["pos"], pos.x, pos.y, pos.z);
		glUniform2f(raytrace.uniform_locations["pyramid"], width / (float)height, 1.f);
		glUniform1f(raytrace.uniform_locations["time"], time);
		//glUniform1f(raytrace.uniform_locations["rand"], (rand()%1024)/1024.f);
		glDispatchCompute(compute_width, compute_height, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glUseProgram(screen.id);

		glBindTexture(GL_TEXTURE_2D, screen_texture);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

gl_drawer gld;

void center_cursor(HWND wnd) {
	if (cursor_disable) {
		POINT point;
		if (GetCursorPos(&point)) {
			ScreenToClient(wnd, &point);

			restore_cursor_x = point.x;
			restore_cursor_y = point.y;
		}
		RECT r;
		GetClientRect(wnd, &r);
		POINT pt = { r.right / 2, r.bottom / 2 };
		ClientToScreen(wnd, &pt);
		SetCursorPos(pt.x, pt.y);
		pt = { 0, 0 };
		ClientToScreen(wnd, &pt);
		SetRect(&r, pt.x + 2, pt.y + 2, pt.x + r.right - 2, pt.y + r.bottom - 2);
		ClipCursor(&r);
	}
	else {
		ClipCursor(NULL);
		POINT pt = { (int32_t)restore_cursor_x, (int32_t)restore_cursor_y };
		ClientToScreen(wnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
	case WM_SIZE:
		RECT r;
		GetClientRect(wnd, &r);
		width = r.right - r.left;
		height = r.bottom - r.top;
		glViewport(0, 0, width, height);
		break;
	case WM_SETCURSOR:
		if (LOWORD(lparam) == HTCLIENT) {
			SetCursor(NULL);
			return TRUE;
		}
		break;
	case WM_KEYDOWN:
		switch (wparam) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case 'L':
			cursor_disable = !cursor_disable;
			center_cursor(wnd);
			break;
		case VK_UP:
			pos_speed *= 1.5f;
			break;
		case VK_DOWN:
			pos_speed /= 1.5f;
			break;
		}
	case WM_MOUSEMOVE: {
		if (!mouse_in) {
			mouse_in = true;

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = wnd;
			tme.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tme);
		}
		if (cursor_disable) {
			RECT r;
			if (!GetClientRect(wnd, &r)) break;
			POINT prev_point = { r.right / 2, r.bottom / 2 };

			POINT point;
			if (!GetCursorPos(&point)) break;
			ScreenToClient(wnd, &point);

			vec3<float> dir_ang = { 0, 0, 0 };

			dir_ang.x = prev_point.y - point.y;
			dir_ang.y = prev_point.x - point.x;

			ang += dir_ang * (ang_speed / min(width, height));
			if (ang.x < -1.570796f) ang.x = -1.570796f;
			else if (ang.x > 1.570796f) ang.x = 1.570796f;
			if (ang.y < 0)         ang.y += 6.283185f;
			else if (ang.y > 6.283185f) ang.y -= 6.283185f;

			ClientToScreen(wnd, &prev_point);
			SetCursorPos(prev_point.x, prev_point.y);
		}
		else {
			POINT point;
			if (GetCursorPos(&point)) {
				ScreenToClient(wnd, &point);

				restore_cursor_x = point.x;
				restore_cursor_y = point.y;
			}
		}
	} break;
	case WM_MOUSELEAVE:
		mouse_in = false;
	}
	return DefWindowProcW(wnd, msg, wparam, lparam);
}

typedef struct k_state {
	bool pressed;
	bool released;
	bool held;
}k_state;

HGLRC get_gl(HDC dc) {
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24,
		8,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};
	SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);
	HGLRC temp = wglCreateContext(dc);
	wglMakeCurrent(dc, temp);

	typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB) {
		MessageBoxW(nullptr, TEXT("ОШИБКА ЕРОР"), TEXT("Не поддерживается новый опенгл."), MB_ICONERROR | MB_OK);
		return temp;
	}

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x00000002

	int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		//WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0
	};

	HGLRC modernContext = wglCreateContextAttribsARB(dc, 0, attribs);

	if (modernContext) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(temp);
		wglMakeCurrent(dc, modernContext);
		return modernContext;
	}

	MessageBoxW(nullptr, TEXT("ОШИБКА ЕРОР"), TEXT("Я не знаю что пошло не так."), MB_ICONERROR | MB_OK);
	return temp;
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmd_line, int cmd_show) {

	srand(time(0));




	WNDCLASSEXW wc;

	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = inst;
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("64-tree");
	wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

	if (!RegisterClassExW(&wc)) {
		MessageBoxW(nullptr, TEXT("ОШИБКА ЕРОР"), TEXT("Не зарегистрировал класс"), MB_ICONERROR | MB_OK);
		return 0;
	}




	HWND wnd = CreateWindowExW(0, wc.lpszClassName, TEXT("64-tree"), WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, 512, 512, 
		nullptr, nullptr, wc.hInstance, nullptr);

	if (!(wnd && IsWindow(wnd))) {
		MessageBoxW(nullptr, TEXT("ОШИБКА ЕРОР"), TEXT("Не создал окно"), MB_ICONERROR | MB_OK);
		return 0;
	}




	HDC hdc = GetDC(wnd);
	HGLRC hrc = get_gl(hdc);
	high_gl_init();




	gld.init();



	shader_program cursor_prog;

	cursor_prog.init(true,
		compile_shader(GL_VERTEX_SHADER, shader_source_cursor_vert, L"вершинный курсора"),
		compile_shader(GL_FRAGMENT_SHADER, shader_source_cursor_frag, L"фрагментный курсора")
	);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);




	std::chrono::high_resolution_clock::time_point old_time = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point new_time = std::chrono::high_resolution_clock::now();

	double dt = 0.00001f;




	ShowWindow(wnd, cmd_show);
	center_cursor(wnd);




	vec3<float> dir_pos;

	k_state m_keys[256];
	int16_t m_keyOldState[256];
	int16_t m_keyNewState[256];

	for (uint16_t i(256); i--;) {
		m_keyNewState[i] = GetAsyncKeyState(i);

		if (m_keyNewState[i] & 0x8000) {
			m_keys[i].released = false;
			m_keys[i].held = true;
			m_keys[i].pressed = true;
		} else {
			m_keys[i].released = true;
			m_keys[i].held = false;
			m_keys[i].pressed = false;
		}

		m_keyOldState[i] = m_keyNewState[i];
	}




	MSG msg;
	do {
		for (uint16_t i(256); i--;) {
			m_keyNewState[i] = GetAsyncKeyState(i);

			m_keys[i].pressed = false;
			m_keys[i].released = false;

			if (m_keyNewState[i] != m_keyOldState[i])
				if (m_keyNewState[i] & 0x8000) {
					m_keys[i].pressed = !m_keys[i].held;
					m_keys[i].held = true;
				} else {
					m_keys[i].released = true;
					m_keys[i].held = false;
				}

			m_keyOldState[i] = m_keyNewState[i];
		}

		new_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - old_time).count();
		old_time = new_time;

		dir_pos.x = m_keys['D'].held - m_keys['A'].held;
		dir_pos.z = m_keys['W'].held - m_keys['S'].held;
		float l = dir_pos.x * dir_pos.x + dir_pos.z * dir_pos.z;
		if (l > 0) {
			l = 1.f / sqrtf(l);
			dir_pos.x *= l;
			dir_pos.z *= l;
			dir_pos = rot_zx(-ang.y) * dir_pos;
		}
		dir_pos.y = m_keys[VK_SPACE].held - m_keys[VK_SHIFT].held;

		pos += dir_pos * (pos_speed * dt);

		hdc = GetDC(wnd);

		glClear(GL_COLOR_BUFFER_BIT);
		gld.draw(hdc);
		glUseProgram(0);
		if (mouse_in && !cursor_disable) {
			glUseProgram(cursor_prog.id);
			glUniform4f(cursor_prog.uniform_locations["color"], 1.f, 0.f, 0.f, 1.f);
			glUniform2f(cursor_prog.uniform_locations["position"], restore_cursor_x, height - restore_cursor_y);
			glUniform2f(cursor_prog.uniform_locations["window"], width, height);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		SwapBuffers(hdc);
		//std::wstring s;
		//s = std::to_wstring(dt) + L"tpf, " + std::to_wstring(1.0 / dt) + L" fps";
		//TextOutW(hdc, 0, 20, s.c_str(), s.length());
		ReleaseDC(wnd, hdc);

		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			if (msg.message == WM_QUIT)
				goto fin;
		}
	} while (msg.message != WM_QUIT);

fin:

	gld.finit();

	if (hrc) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hrc);
	}

	if (wnd && IsWindow(wnd)) DestroyWindow(wnd);

	return msg.wParam;
}