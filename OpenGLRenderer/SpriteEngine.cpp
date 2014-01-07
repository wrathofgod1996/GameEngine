#define GLM_SWIZZLE
#include "SpriteEngine.h"
#include "VertexStructures.h"

#include <cassert>
#include <algorithm>

SpriteEngine::SpriteEngine(ResourceManager* pRm, VertexBuffer* pVertexStruct, Camera* pCam) :
	m_pRM(pRm), m_pVertexBuffer(pVertexStruct), m_iCurrentLength(0), m_pCamera(pCam)
{
}

SpriteEngine::~SpriteEngine()
{
}

void SpriteEngine::SetCamera(Camera* pCam)
{
	m_pCamera = pCam;
}

void SpriteEngine::DrawSprite(const std::string& tech,
							  const std::string& texture,
							  const glm::mat4& transformation,
							  const glm::vec4& color,
							  const glm::vec2& tiling,
							  unsigned int iCellId
							  )
{
	if(m_iCurrentLength < m_pVertexBuffer->GetLength())
	{
		IResource* pShader = m_pRM->GetResource(tech);
		IResource* pTex = m_pRM->GetResource(texture);

		if( pShader != nullptr && pTex != nullptr)
		{
			int iZorder = { (int)floor(transformation[3].z) };
			m_spriteLayers[iZorder][tech][texture].push_back({ transformation, color, tiling, iCellId });
			++m_iCurrentLength;
		}
	}
	else
	{
		// todo: create some error msg
	}
}

void SpriteEngine::FillVertexBuffer()
{
	m_pVertexBuffer->Bind();
	VertexPCT* pVert = static_cast<VertexPCT*>(glMapBufferRange(GL_ARRAY_BUFFER , 0, m_pVertexBuffer->GetLength(), GL_MAP_WRITE_BIT));

	// Loop over all of the layers
	for (auto& layerIter : m_spriteLayers)
	{
		// Loop over all sprites
		for (auto& techIter : layerIter.second)
		{
			for (auto& textureIter : techIter.second)
			{
				TextureInfo texInfo;
				m_pRM->GetTextureInfo(textureIter.first, texInfo);

				// write all sprites that use the same tech and texture to the buffer
				for (auto& iter : textureIter.second)
				{
					unsigned int x = { iter.iCellId % texInfo.uiCellsWidth };
					unsigned int y = { iter.iCellId / texInfo.uiCellsWidth };

					// tex coords
					glm::vec2 topLeft(x / (float)(texInfo.uiCellsWidth),y / (float)(texInfo.uiCellsHeight));
					glm::vec2 bottomRight((x+1) / (float)(texInfo.uiCellsWidth),(y+1) / (float)(texInfo.uiCellsHeight));

					// filling in the vertices
					pVert[0].pos = (iter.T * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f)).xyz();
					pVert[0].color = iter.color;
					pVert[0].tex = topLeft * iter.tiling;
					
					pVert[1].pos = (iter.T * glm::vec4(-0.5f, -0.5f, 0.0, 1.0f)).xyz();
					pVert[1].color = iter.color;
					pVert[1].tex = glm::vec2(topLeft.x, bottomRight.y) * iter.tiling;
					
					pVert[2].pos = (iter.T * glm::vec4(0.5f, 0.5f, 0.0, 1.0f)).xyz();
					pVert[2].color = iter.color;
					pVert[2].tex = glm::vec2(bottomRight.x, topLeft.y) * iter.tiling;

					pVert[3].pos = (iter.T * glm::vec4(0.5f, -0.5f, 0.0, 1.0f)).xyz();
					pVert[3].color = iter.color;
					pVert[3].tex = bottomRight * iter.tiling;

					pVert += 4;
				}
			}
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

}

void SpriteEngine::Render()
{
	// if there is nothing to draw, do nothing
	if(m_spriteLayers.empty())
		return;

	// First we must fill the dynamic vertex buffer with all of the sprites sorted by their tech and texture
	FillVertexBuffer();

	m_pVertexBuffer->BindVAO();

	glActiveTexture(GL_TEXTURE0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	unsigned long uiStartingIndex = 0; // Starting index to the vertex buffer for rendering multiple textures within one shader pass

		// Loop over all of the layers
	for(auto& layerIter : m_spriteLayers)
	{
		// Loop over all sprites with the same tech
		for(auto& techIter : layerIter.second)
		{
			// Apply the shader tech
			const TexturedShader* pShader = static_cast<const TexturedShader*>(m_pRM->GetResource(techIter.first));

			glUseProgram(pShader->GetID());

			// set shader parameters
			glUniformMatrix4fv(pShader->GetMVP(),1,false,&m_pCamera->ViewProj()[0][0]);

			// Render all sprites that use the same tech and texture
			for(auto& spriteIter : techIter.second)
			{
				const Texture* pTexture = static_cast<const Texture*>(m_pRM->GetResource(spriteIter.first));

				glBindTexture(GL_TEXTURE_2D, pTexture->GetID());
				glUniform1i(pShader->GetTextureSamplerID(),0);

				glDrawElements(GL_TRIANGLES, spriteIter.second.size() * 6, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(uiStartingIndex * 12));

				// Increment the index to the dynamic buffer for rendering a new batch of sprites
				uiStartingIndex += spriteIter.second.size();
			}
		}
	}

	glDisable(GL_BLEND);

	m_spriteLayers.clear();
	m_iCurrentLength = 0;
}




