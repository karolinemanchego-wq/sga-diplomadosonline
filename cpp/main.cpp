#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <algorithm>
#include <iomanip>

// 1. JERARQUÍA DE CLASES 

class Persona {
protected:
    std::string cedula;
    std::string nombreCompleto;
    std::string correo;
public:
    Persona(std::string ced, std::string nom, std::string corr) 
        : cedula(ced), nombreCompleto(nom), correo(corr) {}
    virtual ~Persona() {}

    std::string getCedula() const { return cedula; }
    std::string getNombreCompleto() const { return nombreCompleto; }
    std::string getCorreo() const { return correo; }
};

class ProgramaAcademico {
protected:
    std::string nombrePrograma;
    int duracion;
    double notaMinima;
public:
    ProgramaAcademico(std::string nom, int dur, double min) 
        : nombrePrograma(nom), duracion(dur), notaMinima(min) {}
    virtual ~ProgramaAcademico() {}

    std::string getNombrePrograma() const { return nombrePrograma; }

    virtual bool evaluarAprobacion(const std::vector<double>& notas) = 0;
};

class Curso : public ProgramaAcademico {
public:
    Curso() : ProgramaAcademico("Curso", 1, 10.0) {}
    bool evaluarAprobacion(const std::vector<double>& notas) override {
        //  promedio académico real siempre se divide entre 3 evaluaciones fijas
        double suma = 0;
        for(double n : notas) suma += n;
        double promedio = suma / 3.0;
        return promedio >= this->notaMinima;
    }
};

class Diplomado : public ProgramaAcademico {
public:
    Diplomado() : ProgramaAcademico("Diplomado", 1, 14.0) {}
    bool evaluarAprobacion(const std::vector<double>& notas) override {

        double suma = 0;
        for(double n : notas) suma += n;
        double promedio = suma / 3.0;
        return promedio >= this->notaMinima;
    }
};

class Bootcamp : public ProgramaAcademico {
public:
    Bootcamp() : ProgramaAcademico("Bootcamp", 1, 14.0) {}
    bool evaluarAprobacion(const std::vector<double>& notas) override {
        
        bool tieneFaltantes = (notas.size() < 3);
        switch (tieneFaltantes) { case true: return false; }
        
        for(double n : notas) {
            switch(n < 14.0) { case true: return false; }
        }
        return true;
    }
};

class Alumno : public Persona {
private:
    ProgramaAcademico* programa; // EVAL-02: Instancia polimórfica directa asociada
    std::vector<double> notas;
public:
    Alumno(std::string ced, std::string nom, std::string corr, std::string nombreProg, std::vector<double> nts)
        : Persona(ced, nom, corr), notas(nts) {
        
        std::string p = nombreProg;
        std::transform(p.begin(), p.end(), p.begin(), ::tolower);
        
        bool esDiplomado = (p == "diplomado");
        bool esBootcamp = (p == "bootcamp");

        programa = esDiplomado ? static_cast<ProgramaAcademico*>(new Diplomado()) : 
                   (esBootcamp ? static_cast<ProgramaAcademico*>(new Bootcamp()) : 
                                 static_cast<ProgramaAcademico*>(new Curso()));
    }

    ~Alumno() override {
        delete programa; 
    }

    ProgramaAcademico* getPrograma() const { return programa; }
    std::vector<double> getNotas() const { return notas; }
    void setNotas(const std::vector<double>& nuevasNotas) { this->notas = nuevasNotas; }
};

class Profesor : public Persona {
private:
    std::string especialidad;
    std::string materia;
public:
    Profesor(std::string ced, std::string nom, std::string corr, std::string esp, std::string mat)
        : Persona(ced, nom, corr), especialidad(esp), materia(mat) {}
    ~Profesor() override {}

    std::string getEspecialidad() const { return especialidad; }
    std::string getMateria() const { return materia; }
};

struct RegistroHistorial {
    Alumno* alumno;
    std::vector<double> notasAnteriores;
};

// 2. SISTEMA DE GESTIÓN ACADÉMICA (SGA)

class SGA {
private:
    std::vector<Alumno*> listaAlumnos;       
    std::vector<Profesor*> listaProfesores;
    std::queue<Alumno*> colaCertificados;    // Estructura FIFO
    std::stack<RegistroHistorial> pilaHistorialNotas; // Estructura LIFO

public:
    SGA() {
        cargarDesdeArchivos();
    }

    ~SGA() {
        limpiarMemoriaRAM(); 
    }

    void limpiarMemoriaRAM() {

        for(Alumno* a : listaAlumnos) delete a;
        for(Profesor* p : listaProfesores) delete p;
        listaAlumnos.clear();
        listaProfesores.clear();
        
        while(!colaCertificados.empty()) colaCertificados.pop();
        while(!pilaHistorialNotas.empty()) pilaHistorialNotas.pop();
    }

    void cargarDesdeArchivos() {
        for(Alumno* a : listaAlumnos) delete a;
        for(Profesor* p : listaProfesores) delete p;
        listaAlumnos.clear();
        listaProfesores.clear();

        std::ifstream fileAlumnos("alumnos.txt");
        bool strAlumnosOk = fileAlumnos.is_open();
        switch (strAlumnosOk) {
            case true: {
                std::string linea;
                while (std::getline(fileAlumnos, linea)) {
                    std::stringstream ss(linea);
                    std::string token;
                    std::vector<std::string> partes;
                    while (std::getline(ss, token, ',')) {
                        partes.push_back(token);
                    }
                    bool lineaValida = partes.size() >= 4;
                    switch (lineaValida) {
                        case true: {
                            std::string ced = partes[0];
                            std::string nom = partes[1];
                            std::string cor = partes[2];
                            std::string prog = partes[3];
                            std::vector<double> nts;
                            for (size_t i = 4; i < partes.size(); ++i) {
                                switch(!partes[i].empty()) {
                                    case true: nts.push_back(std::stod(partes[i]));
                                }
                            }
                            listaAlumnos.push_back(new Alumno(ced, nom, cor, prog, nts));
                            break;
                        }
                    }
                }
                fileAlumnos.close();
                break;
            }
        }

        // Cargar profesores
        std::ifstream fileProfesores("profesores.txt");
        bool strProfsOk = fileProfesores.is_open();
        switch (strProfsOk) {
            case true: {
                std::string linea;
                while (std::getline(fileProfesores, linea)) {
                    std::stringstream ss(linea);
                    std::string token;
                    std::vector<std::string> partes;
                    while (std::getline(ss, token, ',')) {
                        partes.push_back(token);
                    }
                    bool lineaValida = partes.size() >= 5;
                    switch (lineaValida) {
                        case true:
                            listaProfesores.push_back(new Profesor(partes[0], partes[1], partes[2], partes[3], partes[4]));
                            break;
                    }
                }
                fileProfesores.close();
                break;
            }
        }
    }

    void guardarEstudiantesTxt() {

        std::ofstream file("alumnos.txt");
        for (Alumno* a : listaAlumnos) {
            file << a->getCedula() << "," << a->getNombreCompleto() << "," 
                 << a->getCorreo() << "," << a->getPrograma()->getNombrePrograma();
            for (double n : a->getNotas()) {
                file << "," << n;
            }
            file << "\n";
        }
        file.close();
    }

    void guardarProfesoresTxt() {
        std::ofstream file("profesores.txt");
        for (Profesor* p : listaProfesores) {
            file << p->getCedula() << "," << p->getNombreCompleto() << "," 
                 << p->getCorreo() << "," << p->getEspecialidad() << "," << p->getMateria() << "\n";
        }
        file.close();
    }

    void registrarAlumno(Alumno* alumno) {
        listaAlumnos.push_back(alumno);
        guardarEstudiantesTxt(); 
        std::cout << "Alumno " << alumno->getNombreCompleto() << " registrado con éxito.\n";
    }

    void registrarProfesor(Profesor* profesor) {
        listaProfesores.push_back(profesor);
        guardarProfesoresTxt();
        std::cout << "Profesor " << profesor->getNombreCompleto() << " registrado con éxito.\n";
    }

    void registrarListaNotas(std::string cedula, std::vector<double> nuevasNotas) {
        bool encontrado = false;
        for (Alumno* a : listaAlumnos) {
            bool coincideCedula = (a->getCedula() == cedula);
            switch (coincideCedula) {
                case true: {
                    encontrado = true;
                
                    bool excedeLimite = nuevasNotas.size() > 3;
                    switch (excedeLimite) {
                        case true:
                            std::cout << "Error: No se pueden registrar más de 3 notas para este programa.\n";
                            return;
                        case false:
                        
                            RegistroHistorial reg;
                            reg.alumno = a;
                            reg.notasAnteriores = a->getNotas();
                            pilaHistorialNotas.push(reg);

                            a->setNotas(nuevasNotas);
                            guardarEstudiantesTxt(); 
                            std::cout << "Notas actualizadas con éxito para " << a->getNombreCompleto() << ".\n";
                            return;
                    }
                }
                default: break;
            }
        }
        std::cout << (encontrado ? "" : "Error: Alumno no encontrado.\n");
}

void SGA::deshacerUltimaNota() {
    bool vacia = pilaHistorialNotas.empty();
    switch (vacia) {
        case true:
            std::cout << "No hay cambios de notas para deshacer en la pila.\n";
            break;
        case false: {
        
            RegistroHistorial historial = pilaHistorialNotas.top();
            pilaHistorialNotas.pop();
            historial.alumno->setNotas(historial.notasAnteriores);
            guardarEstudiantesTxt();
            std::cout << "Deshecho: Se restauraron las notas anteriores de " << historial.alumno->getNombreCompleto() << ".\n";
            break;
        }
    }
}

void SGA::generarColaCertificados() {
    while (!colaCertificados.empty()) colaCertificados.pop();
    for (Alumno* a : listaAlumnos) {
        bool aprobado = a->getPrograma()->evaluarAprobacion(a->getNotas());
        switch (aprobado) {
            case true:
                colaCertificados.push(a); 
            default: break;
        }
    }

    std::ofstream bw("certificados_pendientes.txt");
    bw << "=========================================\n";
    bw << "REPORTE DE CERTIFICADOS PENDIENTES\n";
    bw << "=========================================\n";
    bw << "Total de graduandos en cola: " << colaCertificados.size() << "\n\n";
    
    int i = 1;
    std::queue<Alumno*> tempCola = colaCertificados;
    while (!tempCola.empty()) {
        Alumno* est = tempCola.front();
        tempCola.pop();
        double suma = 0;
        for (double n : est->getNotas()) suma += n;
        double prom = suma / 3.0; //  promedio siempre se divide entre 3 evaluaciones programadas
        
        bw << i << ". [" << est->getCedula() << "] " << est->getNombreCompleto() << "\n";
        bw << "   - Programa: " << est->getPrograma()->getNombrePrograma() << "\n";
        bw << "   - Promedio Final: " << std::fixed << std::setprecision(2) << prom << "\n";
        bw << "   - Estatus: APROBADO\n\n";
        i++;
    }
    bw << "=========================================\n";
    bw << "* Fin del reporte - Generado por SGA-DO *\n";
    bw.close();
    std::cout << "Cola generada. " << colaCertificados.size() << " estudiantes pasaron a 'certificados_pendientes.txt'.\n";
}

void SGA::mostrarReporteGeneral() {
    std::cout << "\n========================================\n";
    std::cout << "--- PROFESORES REGISTRADOS ---\n";
    std::cout << "========================================\n";
    bool profsVacios = listaProfesores.empty();
    std::cout << (profsVacios ? "No hay profesores registrados.\n" : "");
    for (Profesor* p : listaProfesores) {
        std::cout << "[" << p->getCedula() << "] " << p->getNombreCompleto() << " - " << p->getEspecialidad() << " (" << p->getMateria() << ")\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "--- REPORTE GENERAL DE ALUMNOS ---\n";
    std::cout << "========================================\n";
    bool alumnosVacios = listaAlumnos.empty();
    switch (alumnosVacios) {
        case true:
            std::cout << "No hay alumnos registrados.\n";
            return;
        default: break;
    }
    for (Alumno* a : listaAlumnos) {

        bool esAprobado = a->getPrograma()->evaluarAprobacion(a->getNotas());
        std::string estatusTexto = esAprobado ? "APROBADO" : "REPROBADO";
        double suma = 0;
        for (double n : a->getNotas()) suma += n;
        double prom = suma / 3.0; //  El promedio siempre se divide entre 3 evaluaciones programadas
        
        std::cout << "• Alumno: " << a->getNombreCompleto() << " -> [" << estatusTexto << "]\n";
        std::cout << "  Cédula: " << a->getCedula() << " | Programa: " << a->getPrograma()->getNombrePrograma() << "\n";
        std::cout << "  Notas: [";
        for (size_t idx = 0; idx < a->getNotas().size(); ++idx) {
            std::cout << a->getNotas()[idx] << (idx + 1 == a->getNotas().size() ? "" : ", ");
        }
        std::cout << "] | Promedio: " << std::fixed << std::setprecision(2) << prom << "\n";
        std::cout << "-----------------------------------\n";
    }
}

void SGA::limpiarMemoriaYSalir() {
    guardarEstudiantesTxt();
    guardarProfesoresTxt();
    limpiarMemoriaRAM(); 
    std::cout << "Sistema cerrado de forma segura. ¡Hasta luego!\n";
}

// 3. MENÚ

int main() {
    SGA sga;
    std::string opcion;
    bool enEjecucion = true;

    while (enEjecucion) {
    
        std::cout << "\n==================================================\n";
        std::cout << "SGA-DO: SISTEMA DIPLOMADOSONLINE\n";
        std::cout << "==================================================\n";
        std::cout << "1. Registrar Alumno\n";
        std::cout << "2. Registrar Profesor\n";
        std::cout << "3. Registrar Notas a un Alumno\n";
        std::cout << "4. Deshacer Ultimo Registro de Nota\n";
        std::cout << "5. Generar Cola de Certificados\n";
        std::cout << "6. Mostrar Reporte General\n";
        std::cout << "7. Salir\n";
        std::cout << "==================================================\n";
        std::cout << "Seleccione una opcion (1-7): ";

        if (!std::getline(std::cin, opcion)) break;

        bool opcionVacia = opcion.empty();
        switch (opcionVacia) {
            case true:
                std::cout << "Error: Ingrese un valor numérico válido del 1 al 7.\n";
                opcion = "PROCESAR_SIGUIENTE";
                break;
            default: break;
        }

        char optChar = (opcion.size() == 1) ? opcion[0] : 'X';
        optChar = (opcion == "PROCESAR_SIGUIENTE") ? 'P' : optChar;

        switch (optChar) {
            case 'P': break;
            case '1': {
                std::string ced, nom, cor, prog;
                std::cout << "Cédula: "; std::getline(std::cin, ced);
                std::cout << "Nombre completo: "; std::getline(std::cin, nom);
                std::cout << "Correo: "; std::getline(std::cin, cor);
                std::cout << "Programa (Curso / Diplomado / Bootcamp): "; std::getline(std::cin, prog);
                sga.registrarAlumno(new Alumno(ced, nom, cor, prog, std::vector<double>()));
                break;
            }
            case '2': {
                std::string ced, nom, cor, esp, mat;
                std::cout << "Cédula: "; std::getline(std::cin, ced);
                std::cout << "Nombre completo: "; std::getline(std::cin, nom);
                std::cout << "Correo: "; std::getline(std::cin, cor);
                std::cout << "Especialidad: "; std::getline(std::cin, esp);
                std::cout << "Materia: "; std::getline(std::cin, mat);
                sga.registrarProfesor(new Profesor(ced, nom, cor, esp, mat));
                break;
            }
            case '3': {
                std::string ced, entradaNotas;
                std::cout << "Cédula del alumno: "; std::getline(std::cin, ced);
                std::cout << "Ingrese las notas separadas por comas (ej: 18,16,20): ";
                std::getline(std::cin, entradaNotas);
                try {
                    std::stringstream ss(entradaNotas);
                    std::string token;
                    std::vector<double> listaDeNotas;
                    bool errorParsing = false;
                    bool errorRango = false;
                    while (std::getline(ss, token, ',')) {
                        switch (!token.empty()) {
                            case true: {
                                size_t idxParsed = 0;
                                try {
                                    double val = std::stod(token, &idxParsed);
                                    switch (idxParsed < token.size()) { case true: errorParsing = true; }
                                    errorRango = (val < 0.0 || val > 20.0) ? true : errorRango;
                                    listaDeNotas.push_back(val);
                                } catch (...) {
                                    errorParsing = true;
                                }
                                break;
                            }
                            default: break;
                        }
                    }
                    switch (errorParsing) {
                        case true:
                            std::cout << "Error: Ingrese un valor numérico válido.\n";
                            break;
                        case false: {
                            switch (errorRango) {
                                case true:
                                    std::cout << "Error: Las notas deben estar en el rango de 0 a 20.\n";
                                    break;
                                case false:
                                    switch (listaDeNotas.empty()) {
                                        case true:  
                                            std::cout << "Error: No ingresó ninguna nota.\n"; 
                                            break;
                                        case false: 
                                            sga.registrarListaNotas(ced, listaDeNotas); 
                                            break;
                                    }
                                    break;
                            }
                            break;
                        }
                    }
                } catch (...) {
                    std::cout << "Error: Ingrese un valor numérico válido.\n";
                }
                break;
            }
            case '4':
                sga.deshacerUltimaNota();
                break;
            case '5':
                sga.generarColaCertificados();
                break;
            case '6':
                sga.mostrarReporteGeneral();
                break;
            case '7':
                sga.limpiarMemoriaYSalir();
                enEjecucion = false;
                break;
            default:
                std::cout << "Error: Ingrese un valor numérico válido del 1 al 7.\n";
                break;
        }
    }
    return 0;
}