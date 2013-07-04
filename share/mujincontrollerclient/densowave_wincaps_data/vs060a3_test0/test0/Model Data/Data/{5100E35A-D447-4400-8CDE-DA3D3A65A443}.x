xof 0302txt 0064
template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Boolean {
 <4885AE61-78E8-11cf-8F52-0040333594A3>
 WORD truefalse;
}

template Boolean2d {
 <4885AE63-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template MaterialWrap {
 <4885AE60-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshFaceWraps {
 <4885AE62-78E8-11cf-8F52-0040333594A3>
 DWORD nFaceWrapValues;
 Boolean2d faceWrapValues;
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

Header{
1;
0;
1;
}

Mesh {
 274;
 0.50000;0.50000;0.00000;,
 0.46194;0.50000;0.19134;,
 0.46194;0.43750;0.19134;,
 0.50000;0.43750;0.00000;,
 0.35355;0.50000;0.35355;,
 0.35355;0.43750;0.35355;,
 0.19134;0.50000;0.46194;,
 0.19134;0.43750;0.46194;,
 -0.00000;0.50000;0.50000;,
 -0.00000;0.43750;0.50000;,
 -0.19134;0.50000;0.46194;,
 -0.19134;0.43750;0.46194;,
 -0.35355;0.50000;0.35355;,
 -0.35355;0.43750;0.35355;,
 -0.46194;0.50000;0.19134;,
 -0.46194;0.43750;0.19134;,
 -0.50000;0.50000;0.00000;,
 -0.50000;0.43750;0.00000;,
 -0.46194;0.50000;-0.19134;,
 -0.46194;0.43750;-0.19134;,
 -0.35355;0.50000;-0.35355;,
 -0.35355;0.43750;-0.35355;,
 -0.19134;0.50000;-0.46194;,
 -0.19134;0.43750;-0.46194;,
 0.00000;0.50000;-0.50000;,
 0.00000;0.43750;-0.50000;,
 0.19134;0.50000;-0.46194;,
 0.19134;0.43750;-0.46194;,
 0.35355;0.50000;-0.35355;,
 0.35355;0.43750;-0.35355;,
 0.46194;0.50000;-0.19134;,
 0.46194;0.43750;-0.19134;,
 0.46194;0.37500;0.19134;,
 0.50000;0.37500;0.00000;,
 0.35355;0.37500;0.35355;,
 0.19134;0.37500;0.46194;,
 -0.00000;0.37500;0.50000;,
 -0.19134;0.37500;0.46194;,
 -0.35355;0.37500;0.35355;,
 -0.46194;0.37500;0.19134;,
 -0.50000;0.37500;0.00000;,
 -0.46194;0.37500;-0.19134;,
 -0.35355;0.37500;-0.35355;,
 -0.19134;0.37500;-0.46194;,
 0.00000;0.37500;-0.50000;,
 0.19134;0.37500;-0.46194;,
 0.35355;0.37500;-0.35355;,
 0.46194;0.37500;-0.19134;,
 0.46194;0.31250;0.19134;,
 0.50000;0.31250;0.00000;,
 0.35355;0.31250;0.35355;,
 0.19134;0.31250;0.46194;,
 -0.00000;0.31250;0.50000;,
 -0.19134;0.31250;0.46194;,
 -0.35355;0.31250;0.35355;,
 -0.46194;0.31250;0.19134;,
 -0.50000;0.31250;0.00000;,
 -0.46194;0.31250;-0.19134;,
 -0.35355;0.31250;-0.35355;,
 -0.19134;0.31250;-0.46194;,
 0.00000;0.31250;-0.50000;,
 0.19134;0.31250;-0.46194;,
 0.35355;0.31250;-0.35355;,
 0.46194;0.31250;-0.19134;,
 0.46194;0.25000;0.19134;,
 0.50000;0.25000;0.00000;,
 0.35355;0.25000;0.35355;,
 0.19134;0.25000;0.46194;,
 -0.00000;0.25000;0.50000;,
 -0.19134;0.25000;0.46194;,
 -0.35355;0.25000;0.35355;,
 -0.46194;0.25000;0.19134;,
 -0.50000;0.25000;0.00000;,
 -0.46194;0.25000;-0.19134;,
 -0.35355;0.25000;-0.35355;,
 -0.19134;0.25000;-0.46194;,
 0.00000;0.25000;-0.50000;,
 0.19134;0.25000;-0.46194;,
 0.35355;0.25000;-0.35355;,
 0.46194;0.25000;-0.19134;,
 0.46194;0.18750;0.19134;,
 0.50000;0.18750;0.00000;,
 0.35355;0.18750;0.35355;,
 0.19134;0.18750;0.46194;,
 -0.00000;0.18750;0.50000;,
 -0.19134;0.18750;0.46194;,
 -0.35355;0.18750;0.35355;,
 -0.46194;0.18750;0.19134;,
 -0.50000;0.18750;0.00000;,
 -0.46194;0.18750;-0.19134;,
 -0.35355;0.18750;-0.35355;,
 -0.19134;0.18750;-0.46194;,
 0.00000;0.18750;-0.50000;,
 0.19134;0.18750;-0.46194;,
 0.35355;0.18750;-0.35355;,
 0.46194;0.18750;-0.19134;,
 0.46194;0.12500;0.19134;,
 0.50000;0.12500;0.00000;,
 0.35355;0.12500;0.35355;,
 0.19134;0.12500;0.46194;,
 -0.00000;0.12500;0.50000;,
 -0.19134;0.12500;0.46194;,
 -0.35355;0.12500;0.35355;,
 -0.46194;0.12500;0.19134;,
 -0.50000;0.12500;0.00000;,
 -0.46194;0.12500;-0.19134;,
 -0.35355;0.12500;-0.35355;,
 -0.19134;0.12500;-0.46194;,
 0.00000;0.12500;-0.50000;,
 0.19134;0.12500;-0.46194;,
 0.35355;0.12500;-0.35355;,
 0.46194;0.12500;-0.19134;,
 0.46194;0.06250;0.19134;,
 0.50000;0.06250;0.00000;,
 0.35355;0.06250;0.35355;,
 0.19134;0.06250;0.46194;,
 -0.00000;0.06250;0.50000;,
 -0.19134;0.06250;0.46194;,
 -0.35355;0.06250;0.35355;,
 -0.46194;0.06250;0.19134;,
 -0.50000;0.06250;0.00000;,
 -0.46194;0.06250;-0.19134;,
 -0.35355;0.06250;-0.35355;,
 -0.19134;0.06250;-0.46194;,
 0.00000;0.06250;-0.50000;,
 0.19134;0.06250;-0.46194;,
 0.35355;0.06250;-0.35355;,
 0.46194;0.06250;-0.19134;,
 0.46194;0.00000;0.19134;,
 0.50000;0.00000;0.00000;,
 0.35355;0.00000;0.35355;,
 0.19134;0.00000;0.46194;,
 -0.00000;0.00000;0.50000;,
 -0.19134;0.00000;0.46194;,
 -0.35355;0.00000;0.35355;,
 -0.46194;0.00000;0.19134;,
 -0.50000;0.00000;0.00000;,
 -0.46194;0.00000;-0.19134;,
 -0.35355;0.00000;-0.35355;,
 -0.19134;0.00000;-0.46194;,
 0.00000;0.00000;-0.50000;,
 0.19134;0.00000;-0.46194;,
 0.35355;0.00000;-0.35355;,
 0.46194;0.00000;-0.19134;,
 0.46194;-0.06250;0.19134;,
 0.50000;-0.06250;0.00000;,
 0.35355;-0.06250;0.35355;,
 0.19134;-0.06250;0.46194;,
 -0.00000;-0.06250;0.50000;,
 -0.19134;-0.06250;0.46194;,
 -0.35355;-0.06250;0.35355;,
 -0.46194;-0.06250;0.19134;,
 -0.50000;-0.06250;0.00000;,
 -0.46194;-0.06250;-0.19134;,
 -0.35355;-0.06250;-0.35355;,
 -0.19134;-0.06250;-0.46194;,
 0.00000;-0.06250;-0.50000;,
 0.19134;-0.06250;-0.46194;,
 0.35355;-0.06250;-0.35355;,
 0.46194;-0.06250;-0.19134;,
 0.46194;-0.12500;0.19134;,
 0.50000;-0.12500;0.00000;,
 0.35355;-0.12500;0.35355;,
 0.19134;-0.12500;0.46194;,
 -0.00000;-0.12500;0.50000;,
 -0.19134;-0.12500;0.46194;,
 -0.35355;-0.12500;0.35355;,
 -0.46194;-0.12500;0.19134;,
 -0.50000;-0.12500;0.00000;,
 -0.46194;-0.12500;-0.19134;,
 -0.35355;-0.12500;-0.35355;,
 -0.19134;-0.12500;-0.46194;,
 0.00000;-0.12500;-0.50000;,
 0.19134;-0.12500;-0.46194;,
 0.35355;-0.12500;-0.35355;,
 0.46194;-0.12500;-0.19134;,
 0.46194;-0.18750;0.19134;,
 0.50000;-0.18750;0.00000;,
 0.35355;-0.18750;0.35355;,
 0.19134;-0.18750;0.46194;,
 -0.00000;-0.18750;0.50000;,
 -0.19134;-0.18750;0.46194;,
 -0.35355;-0.18750;0.35355;,
 -0.46194;-0.18750;0.19134;,
 -0.50000;-0.18750;0.00000;,
 -0.46194;-0.18750;-0.19134;,
 -0.35355;-0.18750;-0.35355;,
 -0.19134;-0.18750;-0.46194;,
 0.00000;-0.18750;-0.50000;,
 0.19134;-0.18750;-0.46194;,
 0.35355;-0.18750;-0.35355;,
 0.46194;-0.18750;-0.19134;,
 0.46194;-0.25000;0.19134;,
 0.50000;-0.25000;0.00000;,
 0.35355;-0.25000;0.35355;,
 0.19134;-0.25000;0.46194;,
 -0.00000;-0.25000;0.50000;,
 -0.19134;-0.25000;0.46194;,
 -0.35355;-0.25000;0.35355;,
 -0.46194;-0.25000;0.19134;,
 -0.50000;-0.25000;0.00000;,
 -0.46194;-0.25000;-0.19134;,
 -0.35355;-0.25000;-0.35355;,
 -0.19134;-0.25000;-0.46194;,
 0.00000;-0.25000;-0.50000;,
 0.19134;-0.25000;-0.46194;,
 0.35355;-0.25000;-0.35355;,
 0.46194;-0.25000;-0.19134;,
 0.46194;-0.31250;0.19134;,
 0.50000;-0.31250;0.00000;,
 0.35355;-0.31250;0.35355;,
 0.19134;-0.31250;0.46194;,
 -0.00000;-0.31250;0.50000;,
 -0.19134;-0.31250;0.46194;,
 -0.35355;-0.31250;0.35355;,
 -0.46194;-0.31250;0.19134;,
 -0.50000;-0.31250;0.00000;,
 -0.46194;-0.31250;-0.19134;,
 -0.35355;-0.31250;-0.35355;,
 -0.19134;-0.31250;-0.46194;,
 0.00000;-0.31250;-0.50000;,
 0.19134;-0.31250;-0.46194;,
 0.35355;-0.31250;-0.35355;,
 0.46194;-0.31250;-0.19134;,
 0.46194;-0.37500;0.19134;,
 0.50000;-0.37500;0.00000;,
 0.35355;-0.37500;0.35355;,
 0.19134;-0.37500;0.46194;,
 -0.00000;-0.37500;0.50000;,
 -0.19134;-0.37500;0.46194;,
 -0.35355;-0.37500;0.35355;,
 -0.46194;-0.37500;0.19134;,
 -0.50000;-0.37500;0.00000;,
 -0.46194;-0.37500;-0.19134;,
 -0.35355;-0.37500;-0.35355;,
 -0.19134;-0.37500;-0.46194;,
 0.00000;-0.37500;-0.50000;,
 0.19134;-0.37500;-0.46194;,
 0.35355;-0.37500;-0.35355;,
 0.46194;-0.37500;-0.19134;,
 0.46194;-0.43750;0.19134;,
 0.50000;-0.43750;0.00000;,
 0.35355;-0.43750;0.35355;,
 0.19134;-0.43750;0.46194;,
 -0.00000;-0.43750;0.50000;,
 -0.19134;-0.43750;0.46194;,
 -0.35355;-0.43750;0.35355;,
 -0.46194;-0.43750;0.19134;,
 -0.50000;-0.43750;0.00000;,
 -0.46194;-0.43750;-0.19134;,
 -0.35355;-0.43750;-0.35355;,
 -0.19134;-0.43750;-0.46194;,
 0.00000;-0.43750;-0.50000;,
 0.19134;-0.43750;-0.46194;,
 0.35355;-0.43750;-0.35355;,
 0.46194;-0.43750;-0.19134;,
 0.46194;-0.50000;0.19134;,
 0.50000;-0.50000;0.00000;,
 0.35355;-0.50000;0.35355;,
 0.19134;-0.50000;0.46194;,
 -0.00000;-0.50000;0.50000;,
 -0.19134;-0.50000;0.46194;,
 -0.35355;-0.50000;0.35355;,
 -0.46194;-0.50000;0.19134;,
 -0.50000;-0.50000;0.00000;,
 -0.46194;-0.50000;-0.19134;,
 -0.35355;-0.50000;-0.35355;,
 -0.19134;-0.50000;-0.46194;,
 0.00000;-0.50000;-0.50000;,
 0.19134;-0.50000;-0.46194;,
 0.35355;-0.50000;-0.35355;,
 0.46194;-0.50000;-0.19134;,
 0.00000;0.50000;0.00000;,
 0.00000;-0.50000;0.00000;;
 
 288;
 4;0,1,2,3;,
 4;1,4,5,2;,
 4;4,6,7,5;,
 4;6,8,9,7;,
 4;8,10,11,9;,
 4;10,12,13,11;,
 4;12,14,15,13;,
 4;14,16,17,15;,
 4;16,18,19,17;,
 4;18,20,21,19;,
 4;20,22,23,21;,
 4;22,24,25,23;,
 4;24,26,27,25;,
 4;26,28,29,27;,
 4;28,30,31,29;,
 4;30,0,3,31;,
 4;3,2,32,33;,
 4;2,5,34,32;,
 4;5,7,35,34;,
 4;7,9,36,35;,
 4;9,11,37,36;,
 4;11,13,38,37;,
 4;13,15,39,38;,
 4;15,17,40,39;,
 4;17,19,41,40;,
 4;19,21,42,41;,
 4;21,23,43,42;,
 4;23,25,44,43;,
 4;25,27,45,44;,
 4;27,29,46,45;,
 4;29,31,47,46;,
 4;31,3,33,47;,
 4;33,32,48,49;,
 4;32,34,50,48;,
 4;34,35,51,50;,
 4;35,36,52,51;,
 4;36,37,53,52;,
 4;37,38,54,53;,
 4;38,39,55,54;,
 4;39,40,56,55;,
 4;40,41,57,56;,
 4;41,42,58,57;,
 4;42,43,59,58;,
 4;43,44,60,59;,
 4;44,45,61,60;,
 4;45,46,62,61;,
 4;46,47,63,62;,
 4;47,33,49,63;,
 4;49,48,64,65;,
 4;48,50,66,64;,
 4;50,51,67,66;,
 4;51,52,68,67;,
 4;52,53,69,68;,
 4;53,54,70,69;,
 4;54,55,71,70;,
 4;55,56,72,71;,
 4;56,57,73,72;,
 4;57,58,74,73;,
 4;58,59,75,74;,
 4;59,60,76,75;,
 4;60,61,77,76;,
 4;61,62,78,77;,
 4;62,63,79,78;,
 4;63,49,65,79;,
 4;65,64,80,81;,
 4;64,66,82,80;,
 4;66,67,83,82;,
 4;67,68,84,83;,
 4;68,69,85,84;,
 4;69,70,86,85;,
 4;70,71,87,86;,
 4;71,72,88,87;,
 4;72,73,89,88;,
 4;73,74,90,89;,
 4;74,75,91,90;,
 4;75,76,92,91;,
 4;76,77,93,92;,
 4;77,78,94,93;,
 4;78,79,95,94;,
 4;79,65,81,95;,
 4;81,80,96,97;,
 4;80,82,98,96;,
 4;82,83,99,98;,
 4;83,84,100,99;,
 4;84,85,101,100;,
 4;85,86,102,101;,
 4;86,87,103,102;,
 4;87,88,104,103;,
 4;88,89,105,104;,
 4;89,90,106,105;,
 4;90,91,107,106;,
 4;91,92,108,107;,
 4;92,93,109,108;,
 4;93,94,110,109;,
 4;94,95,111,110;,
 4;95,81,97,111;,
 4;97,96,112,113;,
 4;96,98,114,112;,
 4;98,99,115,114;,
 4;99,100,116,115;,
 4;100,101,117,116;,
 4;101,102,118,117;,
 4;102,103,119,118;,
 4;103,104,120,119;,
 4;104,105,121,120;,
 4;105,106,122,121;,
 4;106,107,123,122;,
 4;107,108,124,123;,
 4;108,109,125,124;,
 4;109,110,126,125;,
 4;110,111,127,126;,
 4;111,97,113,127;,
 4;113,112,128,129;,
 4;112,114,130,128;,
 4;114,115,131,130;,
 4;115,116,132,131;,
 4;116,117,133,132;,
 4;117,118,134,133;,
 4;118,119,135,134;,
 4;119,120,136,135;,
 4;120,121,137,136;,
 4;121,122,138,137;,
 4;122,123,139,138;,
 4;123,124,140,139;,
 4;124,125,141,140;,
 4;125,126,142,141;,
 4;126,127,143,142;,
 4;127,113,129,143;,
 4;129,128,144,145;,
 4;128,130,146,144;,
 4;130,131,147,146;,
 4;131,132,148,147;,
 4;132,133,149,148;,
 4;133,134,150,149;,
 4;134,135,151,150;,
 4;135,136,152,151;,
 4;136,137,153,152;,
 4;137,138,154,153;,
 4;138,139,155,154;,
 4;139,140,156,155;,
 4;140,141,157,156;,
 4;141,142,158,157;,
 4;142,143,159,158;,
 4;143,129,145,159;,
 4;145,144,160,161;,
 4;144,146,162,160;,
 4;146,147,163,162;,
 4;147,148,164,163;,
 4;148,149,165,164;,
 4;149,150,166,165;,
 4;150,151,167,166;,
 4;151,152,168,167;,
 4;152,153,169,168;,
 4;153,154,170,169;,
 4;154,155,171,170;,
 4;155,156,172,171;,
 4;156,157,173,172;,
 4;157,158,174,173;,
 4;158,159,175,174;,
 4;159,145,161,175;,
 4;161,160,176,177;,
 4;160,162,178,176;,
 4;162,163,179,178;,
 4;163,164,180,179;,
 4;164,165,181,180;,
 4;165,166,182,181;,
 4;166,167,183,182;,
 4;167,168,184,183;,
 4;168,169,185,184;,
 4;169,170,186,185;,
 4;170,171,187,186;,
 4;171,172,188,187;,
 4;172,173,189,188;,
 4;173,174,190,189;,
 4;174,175,191,190;,
 4;175,161,177,191;,
 4;177,176,192,193;,
 4;176,178,194,192;,
 4;178,179,195,194;,
 4;179,180,196,195;,
 4;180,181,197,196;,
 4;181,182,198,197;,
 4;182,183,199,198;,
 4;183,184,200,199;,
 4;184,185,201,200;,
 4;185,186,202,201;,
 4;186,187,203,202;,
 4;187,188,204,203;,
 4;188,189,205,204;,
 4;189,190,206,205;,
 4;190,191,207,206;,
 4;191,177,193,207;,
 4;193,192,208,209;,
 4;192,194,210,208;,
 4;194,195,211,210;,
 4;195,196,212,211;,
 4;196,197,213,212;,
 4;197,198,214,213;,
 4;198,199,215,214;,
 4;199,200,216,215;,
 4;200,201,217,216;,
 4;201,202,218,217;,
 4;202,203,219,218;,
 4;203,204,220,219;,
 4;204,205,221,220;,
 4;205,206,222,221;,
 4;206,207,223,222;,
 4;207,193,209,223;,
 4;209,208,224,225;,
 4;208,210,226,224;,
 4;210,211,227,226;,
 4;211,212,228,227;,
 4;212,213,229,228;,
 4;213,214,230,229;,
 4;214,215,231,230;,
 4;215,216,232,231;,
 4;216,217,233,232;,
 4;217,218,234,233;,
 4;218,219,235,234;,
 4;219,220,236,235;,
 4;220,221,237,236;,
 4;221,222,238,237;,
 4;222,223,239,238;,
 4;223,209,225,239;,
 4;225,224,240,241;,
 4;224,226,242,240;,
 4;226,227,243,242;,
 4;227,228,244,243;,
 4;228,229,245,244;,
 4;229,230,246,245;,
 4;230,231,247,246;,
 4;231,232,248,247;,
 4;232,233,249,248;,
 4;233,234,250,249;,
 4;234,235,251,250;,
 4;235,236,252,251;,
 4;236,237,253,252;,
 4;237,238,254,253;,
 4;238,239,255,254;,
 4;239,225,241,255;,
 4;241,240,256,257;,
 4;240,242,258,256;,
 4;242,243,259,258;,
 4;243,244,260,259;,
 4;244,245,261,260;,
 4;245,246,262,261;,
 4;246,247,263,262;,
 4;247,248,264,263;,
 4;248,249,265,264;,
 4;249,250,266,265;,
 4;250,251,267,266;,
 4;251,252,268,267;,
 4;252,253,269,268;,
 4;253,254,270,269;,
 4;254,255,271,270;,
 4;255,241,257,271;,
 3;272,1,0;,
 3;272,4,1;,
 3;272,6,4;,
 3;272,8,6;,
 3;272,10,8;,
 3;272,12,10;,
 3;272,14,12;,
 3;272,16,14;,
 3;272,18,16;,
 3;272,20,18;,
 3;272,22,20;,
 3;272,24,22;,
 3;272,26,24;,
 3;272,28,26;,
 3;272,30,28;,
 3;272,0,30;,
 3;273,257,256;,
 3;273,256,258;,
 3;273,258,259;,
 3;273,259,260;,
 3;273,260,261;,
 3;273,261,262;,
 3;273,262,263;,
 3;273,263,264;,
 3;273,264,265;,
 3;273,265,266;,
 3;273,266,267;,
 3;273,267,268;,
 3;273,268,269;,
 3;273,269,270;,
 3;273,270,271;,
 3;273,271,257;;
 
 MeshMaterialList {
  1;
  288;
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0;;
  Material {
   0.000000;0.426667;0.015686;1.000000;;
   5.000000;
   0.000000;0.000000;0.000000;;
   0.000000;0.000000;0.000000;;
  }
 }
 MeshNormals {
  19;
  0.000000;1.000000;0.000000;,
  1.000000;0.000000;0.000000;,
  0.923880;0.000000;0.382683;,
  0.707107;0.000000;0.707107;,
  0.382683;0.000000;0.923880;,
  -0.000000;0.000000;1.000000;,
  -0.382683;0.000000;0.923880;,
  -0.707107;0.000000;0.707107;,
  -0.923880;0.000000;0.382683;,
  -1.000000;0.000000;0.000000;,
  -0.923880;0.000000;-0.382683;,
  -0.707107;0.000000;-0.707107;,
  -0.382684;0.000000;-0.923879;,
  -0.000000;0.000000;-1.000000;,
  0.382684;0.000000;-0.923879;,
  0.707107;0.000000;-0.707107;,
  0.923880;0.000000;-0.382683;,
  0.923880;0.000000;-0.382683;,
  0.000000;-1.000000;0.000000;;
  288;
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,16,17,15;,
  4;16,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,17,15;,
  4;17,1,1,17;,
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,9,9,8;,
  4;9,10,10,9;,
  4;10,11,11,10;,
  4;11,12,12,11;,
  4;12,13,13,12;,
  4;13,14,14,13;,
  4;14,15,15,14;,
  4;15,17,16,15;,
  4;17,1,1,16;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;,
  3;18,18,18;;
 }
}
