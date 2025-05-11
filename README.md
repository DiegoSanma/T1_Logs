# T1_Logs

Para ejecutar nuestra tarea, se tiene que primero ejecutar el siguiente comando de docker en la terminal:

<pre>docker build -t my-app .</pre>
<pre>docker run -m 50m -d my-app . </pre>

El primero es para que se compile el código que se creó, y el segundo es para ejecutar el archivo main.cpp, asegurándose que nuestro contenedor solo use 50mb de RAM. Un punto a considerar, es que para cuando QuickSort llega a valores demasiados grandes, la restricción de memoria puede trabar su ejecución. Esto se debe a que para aridade muy grandes, los pivotes que se guardan en memoria son demasiados y se supera ligeramente el límite de memoria. Sin embargo, se puede ver que en el manejo de código y espacio, este se hace correctamente según la restricción de M= 50.

Para poder correr correctamente el archimo main.cpp, se dejaron 4 variables binarias, que determinan que parte de la tara se corre. Las variables son las siguientes:

1.RunAlpha - busca la aridad óptima

2.RunMerge - corre los MergeSort para los arreglos

3.RunQuick - corre los QuickSort para los arreglos

4.RunAll - corre todos los anteriores
