#include "meshrenderer.h"

int MeshRenderer::main_costume_idx = -1;
int MeshRenderer::sub_constume_idx = -1;

MeshRenderer::MeshRenderer() :
    indexBuf(QOpenGLBuffer::IndexBuffer)
{
    mVec_num_costume = QVector<int> {2, 2, 2, 1, 2, 2, 4};
    for(int i=0; i<mVec_num_costume.length(); i++)
    {
        QVector<bool> tmp;
        for(int j=0; j<mVec_num_costume[i]; j++)
        {
            tmp.push_back(false);
        }
        mVec_costume.push_back(tmp);
        mVec_load_costume.push_back(tmp);
    }
    mVec_hatModel_data = QVector<QVector<CommonShaderData>>(mVec_num_costume[0]);
    mVec_hairModel_data = QVector<QVector<CommonShaderData>>(mVec_num_costume[1]);
    mVec_clothModel_data = QVector<QVector<CommonShaderData>>(mVec_num_costume[2]);
    mVec_pantsModel_data = QVector<QVector<CommonShaderData>>(mVec_num_costume[3]);
    mVec_shoesModel_data = QVector<QVector<CommonShaderData>>(mVec_num_costume[4]);
    mVec_partModel_data = QVector<QVector<CommonShaderData>>(mVec_num_costume[5]);
    mVec_clothsetModel_data = QVector<QVector<QVector<CommonShaderData>>>(mVec_num_costume[6]);

    mVec_hatModel_indices = QVector<QVector<GLushort>>(mVec_num_costume[0]);
    mVec_hairModel_indices = QVector<QVector<GLushort>>(mVec_num_costume[1]);
    mVec_clothModel_indices = QVector<QVector<GLushort>>(mVec_num_costume[2]);
    mVec_pantsModel_indices = QVector<QVector<GLushort>>(mVec_num_costume[3]);
    mVec_shoesModel_indices = QVector<QVector<GLushort>>(mVec_num_costume[4]);
    mVec_partModel_indices = QVector<QVector<GLushort>>(mVec_num_costume[5]);
    mVec_clothsetModel_indices = QVector<QVector<QVector<GLushort>>>(mVec_num_costume[6]);

    mVec_Model_filePath = QVector<QVector<QString>> {
                                                     QVector<QString> {"/modelFiles/hat01.obj", "/modelFiles/hat02.obj"},
                                                     QVector<QString> {"/modelFiles/hair01.obj", "/modelFiles/hair01.obj"},
                                                     QVector<QString> {"/modelFiles/cloth01.obj", "/modelFiles/cloth02.obj"},
                                                     QVector<QString> {"/modelFiles/pants01.obj"},
                                                     QVector<QString> {"/modelFiles/shoes01.obj", "/modelFiles/shoes02.obj"},
                                                     QVector<QString> {"/modelFiles/broom.obj", "/modelFiles/skateboard.obj"},
                                                     QVector<QString> {"/modelFiles/ironman1.obj", "/modelFiles/ironman2.obj", "/modelFiles/ironman3.obj", "/modelFiles/ironman4.obj"},
                                                     QVector<QString> {"/modelFiles/luffy_hat.obj", "/modelFiles/luffy_cloth.obj", "/modelFiles/luffy_shorts.obj", "/modelFiles/luffy_slippers.obj"},
                                                     QVector<QString> {"/modelFiles/minion1.obj", "/modelFiles/minion2.obj", "/modelFiles/minion3.obj", "/modelFiles/minion4.obj"},
                                                     QVector<QString> {"/modelFiles/chopper.obj"}};

}

MeshRenderer::~MeshRenderer()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}

void MeshRenderer::initMeshGeometry()
{
    // Every models start index
    int indices_size = 0;
    std::vector<CommonShaderData> datas;
    std::vector<GLushort> indices;
    mVec_texture_order.clear();
    m_nIndex.resize(3);

    //The model data
    for(int i = 0; i<model_data.length(); i++ )
        for(int j=0; j<model_data[i].size(); j++)
            datas.push_back(model_data[i][j]);

    for(int i=0; i<model_indices.length(); i++)
        for(int j=0; j<model_indices[i].size(); j++)
            indices.push_back(model_indices[i][j]);

    indices_size += datas.size();

    num_model = 2;

    // hat
    for(int i=0; i<mVec_hatModel_data.length(); i++)
    {
        if(mVec_costume[0][i])
        {
            mVec_texture_order.push_back(2);
            num_model++;
            // hat data
            for(int j=0; j<mVec_hatModel_data[i].length(); j++)
                datas.push_back(mVec_hatModel_data[i][j]);
            // hat indice
            for(int j=0; j<mVec_hatModel_indices[i].length(); j++)
                indices.push_back(mVec_hatModel_indices[i][j]+indices_size);
            indices_size += mVec_hatModel_data[i].length();
            m_nIndex.push_back(indices.size());
        }
    }

    // hair
    for(int i=0; i<mVec_hairModel_data.length(); i++)
    {
        if(mVec_costume[1][i])
        {
            mVec_texture_order.push_back(3+i);
            num_model++;
            // hair data
            for(int j=0; j<mVec_hairModel_data[i].length(); j++)
                datas.push_back(mVec_hairModel_data[i][j]);
            // hair indice
            for(int j=0; j<mVec_hairModel_indices[i].length(); j++)
                indices.push_back(mVec_hairModel_indices[i][j]+indices_size);
            indices_size += mVec_hairModel_data[i].length();
            m_nIndex.push_back(indices.size());
        }
    }

    // cloth
    for(int i=0; i<mVec_clothModel_data.length(); i++)
    {
        if(mVec_costume[2][i])
        {
            mVec_texture_order.push_back(5);
            num_model++;
            // cloth data
            for(int j=0; j<mVec_clothModel_data[i].length(); j++)
                datas.push_back(mVec_clothModel_data[i][j]);
            // cloth indice
            for(int j=0; j<mVec_clothModel_indices[i].length(); j++)
                indices.push_back(mVec_clothModel_indices[i][j]+indices_size);
            indices_size += mVec_clothModel_data[i].length();
            m_nIndex.push_back(indices.size());
        }
    }

    // pants
    for(int i=0; i<mVec_pantsModel_data.length(); i++)
    {
        if(mVec_costume[3][i])
        {
            mVec_texture_order.push_back(6);
            num_model++;
            // pants data
            for(int j=0; j<mVec_pantsModel_data[i].length(); j++)
                datas.push_back(mVec_pantsModel_data[i][j]);
            // pants indice
            for(int j=0; j<mVec_pantsModel_indices[i].length(); j++)
                indices.push_back(mVec_pantsModel_indices[i][j]+indices_size);
            indices_size += mVec_pantsModel_data[i].length();
            m_nIndex.push_back(indices.size());
        }
    }

    // shoes
    for(int i=0; i<mVec_shoesModel_data.length(); i++)
    {
        if(mVec_costume[4][i])
        {
            mVec_texture_order.push_back(7);
            num_model++;
            // shoes data
            for(int j=0; j<mVec_shoesModel_data[i].length(); j++)
                datas.push_back(mVec_shoesModel_data[i][j]);
            // shoes indice
            for(int j=0; j<mVec_shoesModel_indices[i].length(); j++)
                indices.push_back(mVec_shoesModel_indices[i][j]+indices_size);
            indices_size += mVec_shoesModel_data[i].length();
            m_nIndex.push_back(indices.size());
        }
    }

    // part
    for(int i=0; i<mVec_partModel_data.length(); i++)
    {
        if(mVec_costume[5][i])
        {
            mVec_texture_order.push_back(8+i);
            num_model++;
            // part data
            for(int j=0; j<mVec_partModel_data[i].length(); j++)
                datas.push_back(mVec_partModel_data[i][j]);
            // part indice
            for(int j=0; j<mVec_partModel_indices[i].length(); j++)
                indices.push_back(mVec_partModel_indices[i][j]+indices_size);
            indices_size += mVec_partModel_data[i].length();
            m_nIndex.push_back(indices.size());
        }
    }

    // clothset
    for(int i=0; i<mVec_clothsetModel_data.length(); i++)
    {
        if(mVec_costume[6][i])
        {
            // clothset data
            for(int j=0; j<mVec_clothsetModel_data[i].length(); j++)
            {
                num_model++;

                if(i == 0)
                    mVec_texture_order.push_back(10+j);
                else if(i == 1)
                    mVec_texture_order.push_back(14);
                else if(i == 2)
                    mVec_texture_order.push_back(15+j);
                else if(i == 3)
                    mVec_texture_order.push_back(19);

                for(int k=0; k<mVec_clothsetModel_data[i][j].length(); k++)
                    datas.push_back(mVec_clothsetModel_data[i][j][k]);
            }
            // clothset indice
            for(int j=0; j<mVec_clothsetModel_indices[i].length(); j++)
            {
                for(int k=0; k<mVec_clothsetModel_indices[i][j].length(); k++)
                {
                    indices.push_back(mVec_clothsetModel_indices[i][j][k]+indices_size);
                }
                indices_size += mVec_clothsetModel_data[i][j].length();
                m_nIndex.push_back(indices.size());
            }
        }
    }

    //The costume data
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(datas.data(), datas.size()*sizeof(CommonShaderData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices.data(), indices.size()*sizeof(GLushort));
}

void MeshRenderer::drawMeshGeometry(QOpenGLShaderProgram *program, int models_idx)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    int offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int positionLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(positionLocation);
    program->setAttributeBuffer(positionLocation, GL_FLOAT, offset, 3, sizeof(CommonShaderData));

    offset += sizeof(QVector3D);
    int normalLocation = program->attributeLocation("a_normal");
    program->enableAttributeArray(normalLocation);
    program->setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, sizeof(CommonShaderData));

    offset += sizeof(QVector3D);
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(CommonShaderData));

    // Draw mesh geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLES, m_nIndex[models_idx]-m_nIndex[models_idx-1], GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(GLubyte) * m_nIndex[models_idx-1] * 2));
}

void MeshRenderer::initMeshGeometry( BaseMesh mesh)
{
    model_data = QVector<QVector<CommonShaderData>>(2);
    model_indices = QVector<QVector<GLushort>>(2);
    // Every models start index
    m_nIndex.clear();
    m_nIndex.push_back(0);
    int idx = 0;
    //not face
    for ( BaseMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it )
    {
        bool isFace = false;
        for( size_t i = 0; i<m_expFHandle.size(); i++ )
        {
            if(m_expFHandle[i].idx()==f_it->idx())
            {
                isFace = true;
            }
        }
        if(isFace == false)
        {
            for(BaseMesh::FaceHalfedgeIter fh_it = mesh.fh_iter(*f_it); fh_it.is_valid(); ++fh_it)
            {
                CommonShaderData shaderData;
                BaseMesh::VertexHandle vh = mesh.to_vertex_handle(fh_it);
                BaseMesh::Point point = mesh.point(vh);
                shaderData.position[0] = point[0];
                shaderData.position[1] = point[1];
                shaderData.position[2] = point[2];
                BaseMesh::Normal normal = mesh.normal(vh);
                shaderData.normal[0] = normal[0];
                shaderData.normal[1] = normal[1];
                shaderData.normal[2] = normal[2];
                BaseMesh::TexCoord2D texCoord = mesh.texcoord2D(*fh_it);
                shaderData.texcoord[0] = texCoord[0];
                shaderData.texcoord[1] = texCoord[1];
                model_data[0].push_back(shaderData);
                model_indices[0].push_back(idx++);
            }
        }
    }
    m_nIndex.push_back(idx);
    for ( BaseMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it )
    {
        bool isFace = false;
        for( size_t i = 0; i<m_expFHandle.size(); i++ )
        {
            if(m_expFHandle[i].idx()==f_it->idx())
            {
                isFace = true;
            }
        }
        if(isFace == true)
        {
            for(BaseMesh::FaceHalfedgeIter fh_it = mesh.fh_iter(*f_it); fh_it.is_valid(); ++fh_it)
            {
                CommonShaderData shaderData;
                BaseMesh::VertexHandle vh = mesh.to_vertex_handle(fh_it);
                BaseMesh::Point point = mesh.point(vh);
                shaderData.position[0] = point[0];
                shaderData.position[1] = point[1];
                shaderData.position[2] = point[2];
                BaseMesh::Normal normal = mesh.normal(vh);
                shaderData.normal[0] = normal[0];
                shaderData.normal[1] = normal[1];
                shaderData.normal[2] = normal[2];
                shaderData.texcoord[0] = VertexUVProp[vh.idx()][0];
                shaderData.texcoord[1] = VertexUVProp[vh.idx()][1];
                model_data[1].push_back(shaderData);
                model_indices[1].push_back(idx++);
            }
        }
    }
    m_nIndex.push_back(idx);
}

void MeshRenderer::TriMesh_to_Mesh(TriMesh* mesh)
{
    int model_data_index = 0;
    for ( BaseMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it )
    {
        bool isFace = false;
        for( size_t i = 0; i<m_expFHandle.size(); i++ )
        {
            if(m_expFHandle[i].idx()==f_it->idx())
            {
                isFace = true;
            }
        }
        if(isFace == false)
        {
            for(BaseMesh::FaceHalfedgeIter fh_it = mesh->fh_iter(*f_it); fh_it.is_valid(); ++fh_it)
            {
                BaseMesh::VertexHandle vh = mesh->to_vertex_handle(fh_it);
                BaseMesh::Point point = mesh->point(vh);
                model_data[0][model_data_index].position[0] = point[0];
                model_data[0][model_data_index].position[1] = point[1];
                model_data[0][model_data_index].position[2] = point[2];
                model_data_index++;
            }
        }
    }
    model_data_index = 0;
    for ( BaseMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it )
    {
        bool isFace = false;
        for( size_t i = 0; i<m_expFHandle.size(); i++ )
        {
            if(m_expFHandle[i].idx()==f_it->idx())
            {
                isFace = true;
            }
        }
        if(isFace == true)
        {
            for(BaseMesh::FaceHalfedgeIter fh_it = mesh->fh_iter(*f_it); fh_it.is_valid(); ++fh_it)
            {
                BaseMesh::VertexHandle vh = mesh->to_vertex_handle(fh_it);
                BaseMesh::Point point = mesh->point(vh);
                model_data[1][model_data_index].position[0] = point[0];
                model_data[1][model_data_index].position[1] = point[1];
                model_data[1][model_data_index].position[2] = point[2];
                model_data_index++;
            }
        }
    }
}

void MeshRenderer::initFaceGeometry( std::vector<BaseMesh::FaceHandle> expFH, std::vector< bool > Mapping, std::vector<BaseMesh::Point> VertexUV, std::vector<BaseMesh::Point> f_mask)
{
    m_expFHandle = expFH;
    b_Mapping = Mapping;
    VertexUVProp = VertexUV;
    FaceMask = f_mask;
}

std::vector<BaseMesh::Point> MeshRenderer::Get_uv()
{
    if(VertexUVProp.empty())
    {
        VertexUVProp.push_back(BaseMesh::Point(0,0,0));
        return VertexUVProp;
    }
    else
    {
        return VertexUVProp;
    }
}

std::vector<BaseMesh::Point> MeshRenderer::Get_mask()
{
    return FaceMask;
}
