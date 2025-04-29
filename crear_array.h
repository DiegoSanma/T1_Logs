#ifndef CrearArray_H
#define CrearArray_H

class CrearArray {
private:
    const char* filename;
    int M;
    int X;

public:
    CrearArray(const char* filename, int M,int X);

   const char* getFileName() const;

    int getM() const;

    int getX() const;

    void setX(int x);

    int crearArrayN() const;
};

#endif