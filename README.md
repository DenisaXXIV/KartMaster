<div align="center">
  <h1>KartMaster: Prejmer Circuit Simulator</h1>
<img width="1188" alt="banner" src="https://github.com/DenisaXXIV/KartMaster-Prejmer-Circuit-Simulator/blob/master/resources/banner.png">
</div>

KartMaster: Prejmer Circuit Simulator is a beginner-friendly 3D modeling project in OpenGL, perfect for learning and exploring 3D graphics.


Copyright - [DenisaXXIV](https://github.com/DenisaXXIV) & [Soreanu Dinu](https://github.com/SoreanuDinu)

## About OpenGL

OpenGL (Open Graphics Library) is a powerful and widely-used cross-platform API (Application Programming Interface) for rendering 2D and 3D computer graphics. It provides developers with a set of functions and capabilities to interact with the GPU (Graphics Processing Unit) and create stunning visuals for various applications, including games, simulations, and scientific visualization.

You can find more: [here]([https://www.opengl.org/](https://www.opengl.org/))

## Getting Started for Visual Studio 2022
<ol>
  <li><strong>Ensure you have Visual Studio 2022 installed on your system, </strong>as it provides an excellent development environment for C++ and OpenGL.</li>
  <li><strong>Download GLEW library:</strong> <a href="https://glew.sourceforge.net/">Download GLEW here</a></li>
  <li><strong>Download FreeGLUT library:</strong> <a href="https://freeglut.sourceforge.net/">Download FreeGLUT here</a></li>
  <li><strong>Download GLM library:</strong> <a href="https://glm.g-truc.net/0.9.9/index.html">Download GLM here</a></li>
  <li><strong>Download GLFW library:</strong> <a href="https://www.glfw.org/">Download GLFW here</a></li>
  <li><strong>Download STB image libraries:</strong> <a href="https://github.com/nothings/stb">Download STB image libraries here</a></li>  
  <li><strong>Create a Windows Console C++ Project in VS 2022</strong></li>
  <li><strong>Make an "External" directory:</strong> inside the folder that contains the .sln file create a new directory with the name "External"</li>
  <li><strong>Move the downloaded libraries in External directory:</strong> move unzipped folders to the "External" directory created in the previous step</li>
   <li><strong>Create a Windows Console C++ Project in VS 2022</strong></li>
  <li><strong>Move the .dll files to the directory that contains your classes:</strong> you cand find the .dll files in bin\Release\x64</li>
  <li><strong>Edit your Project Properties in your VS:</strong>
    <ul>
      <li><strong>C/C++ -> General -> Additional Include Directories:</strong> here add the include directories</li>
      <li><strong>Linker -> General -> Additional Library Directories:</strong> here add the lib\Release\x64 directories</li>
      <li><strong>Linker -> Input -> Additional Dependencies:</strong> here add the .dll files added at step 7, but with .lib extension, not .dll</li>
    </ul>
  </li>
</ol>

*You will need to do steps from 3 to 8 for every project you start.*

## Minimum scale
<ol>
  <li><Strong>Multiple Objects:</Strong> Add multiple objects to the scene, like additional cubes, spheres, or custom models. You can manage their positions, rotations, and scales separately.</li>
  <li><Strong>Textures:</Strong>> Apply textures to objects for more realistic rendering. You can load image files and map them onto your 3D objects.</li>
  <li><strong>Lighting:</strong> Implement different lighting models, such as Phong or Blinn-Phong, and add point lights, directional lights, and spotlights to illuminate your scene. Experiment with ambient, diffuse, and specular lighting components.</li>
  <li><strong>Shaders:</strong> Create custom shaders for various visual effects, like reflection, refraction, or bump mapping. This allows you to achieve more advanced rendering techniques.</li>
  <li><strong>Camera Controls:</strong>  Implement camera controls to allow users to pan, zoom, and orbit around the scene. You can create a first-person or third-person camera perspective.</li>

  ## Images

  <img src="https://github.com/DenisaXXIV/KartMaster-Prejmer-Circuit-Simulator/blob/master/resources/image.png" width="850"/>

  <img src="https://github.com/DenisaXXIV/KartMaster-Prejmer-Circuit-Simulator/blob/master/resources/road.png" width="850"/>

