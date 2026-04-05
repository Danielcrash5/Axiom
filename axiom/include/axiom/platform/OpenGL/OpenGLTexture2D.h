#include "axiom/renderer/Texture2D.h"
#include <glad/glad.h>
namespace axiom {
	class OpenGLTexture2D : public Texture2D {
	public:
		OpenGLTexture2D(int width, int height, bool generateMipmaps, TextureWrap wraps, TextureWrap wrapT, TextureFilter filterMin, TextureFilter filterMag);
		~OpenGLTexture2D();

		void Bind(int slot) const;
		uint64_t GetBindlessHandle() const;
		bool SupportsBindless() const;

		void SetData(const void* Data, uint32_t width, uint32_t height, TextureFormat format);
		void LoadFromFile(const std::string& path, bool sRGB, bool HDR, bool is16Bit);

		void SetWrap(TextureWrap wrapS, TextureWrap wrapT);
		void SetFilter(TextureFilter filterMin, TextureFilter filterMag);
		void SetFormat(TextureFormat textureFormat);

		void GenerateMipmaps();

		static std::shared_ptr<OpenGLTexture2D> CreateFromMemory(
			const std::vector<uint8_t>& data,
			const TextureLoadInfo& info
		);


	private:
		GLuint64 m_BindlessHandle;
		GLuint m_RendererID;
		uint32_t m_Width, m_Height;
		bool m_GenerateMipmaps;

		std::vector<uint8_t> m_data;

		TextureWrap m_WrapS, m_WrapT;
		TextureFilter m_FilterMin, m_FilterMag;
		TextureFormat m_TextureFormat;
	};

}