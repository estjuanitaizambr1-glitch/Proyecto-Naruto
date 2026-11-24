**Konoha**

Nombre: Juanita Isabel Zambrano Maldonado
Código: 6014395

Para desarrollar el proyecto utilicé **programación orientada a objetos**, organizando el código mediante una **arquitectura modular** basada en **clases**. En esta estructura apliqué el **encapsulamiento** al mantener los atributos y recursos internos de cada clase separados de los métodos públicos; por ejemplo, en **`Camera`** se administra internamente la posición y orientación de la cámara, mientras que desde **`main.cpp`** se utilizan sus métodos para actualizarla.

También apliqué la **abstracción**, porque cada clase permite realizar una operación sin tener que repetir todos sus procedimientos internos. Por ejemplo, desde **`Scene`** puedo llamar al método **`load`** de **`Model`** para cargar los archivos glTF, sin tener que escribir nuevamente las operaciones de Assimp, texturas y buffers. Finalmente, utilicé la **composición** en la clase **`Scene`**, que contiene tres objetos de tipo **`Model`** y uno de tipo **`BezierCurve`**. Esto permite integrar la aldea, Naruto y el pájaro con sus respectivas transformaciones y cálculos de movimiento.

Organicé las clases de esta manera porque necesitaba que la cámara, la carga de geometría, los shaders y los movimientos funcionaran como componentes independientes, pero conectados desde `main.cpp` y `Scene` para construir y renderizar la escena completa.
