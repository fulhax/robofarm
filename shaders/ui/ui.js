print("initing");
plane = generateplane(50);
uishader = loadshader("ui.vert", "ui.frag", 0, 0, 0);

setdefaultfilter(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
function loop()
{
    beginpass();
    {
        if(KEY_W & PRESSED)
        {
            wireframe(1);
        }

        culling(CULL_NONE);
        clear(0, 0, 0);
        bindshader(uishader);
        bindattribute("in_Position", MESH_FLAG_POSITION);
        bindattribute("in_Uvs", MESH_FLAG_TEXCOORD0);
        bindattribute("in_Color", MESH_FLAG_COLOR0);
        drawmesh(plane);
        bindshader(-1);
        wireframe(0);
    }
    endpass();
}
