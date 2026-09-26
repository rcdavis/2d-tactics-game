#pragma once

struct Camera2D;

namespace Renderer2D {
	bool Init();

	void Shutdown();

	void BeginScene(Camera2D& camera);
	void EndScene();

	void Flush();
}
