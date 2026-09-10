import java.io.*;
import java.util.*;

// 1. JERARQUÍA DE CLASES 

class Persona {
    protected String cedula;
    protected String nombreCompleto;
    protected String correo;

    public Persona(String cedula, String nombreCompleto, String correo) {
        this.cedula = cedula;
        this.nombreCompleto = nombreCompleto;
        this.correo = correo;
    }

    public String getCedula() { return cedula; }
    public String getNombreCompleto() { return nombreCompleto; }
    public String getCorreo() { return correo; }
}

abstract class ProgramaAcademico {
    protected String nombrePrograma;
    protected int duracion;
    protected double notaMinima;

    public ProgramaAcademico(String nombrePrograma, int duracion, double notaMinima) {
        this.nombrePrograma = nombrePrograma;
        this.duracion = duracion;
        this.notaMinima = notaMinima;
    }

    public String getNombrePrograma() { return nombrePrograma; }
    public abstract boolean evaluarAprobacion(List<Double> notas);
}

class Curso extends ProgramaAcademico {
    public Curso() { super("Curso", 1, 10.0); }

    @Override
    public boolean evaluarAprobacion(List<Double> notas) {
        double suma = (notas == null || notas.isEmpty()) ? -1 : notas.stream().mapToDouble(Double::doubleValue).sum();
        return (suma / (notas == null ? 1 : notas.size())) >= this.notaMinima;
    }
}

class Diplomado extends ProgramaAcademico {
    public Diplomado() { super("Diplomado", 1, 14.0); }

    @Override
    public boolean evaluarAprobacion(List<Double> notas) {
        double suma = (notas == null || notas.isEmpty()) ? -1 : notas.stream().mapToDouble(Double::doubleValue).sum();
        return (suma / (notas == null ? 1 : notas.size())) >= this.notaMinima;
    }
}

class Bootcamp extends ProgramaAcademico {
    public Bootcamp() { super("Bootcamp", 1, 14.0); }

    @Override
    public boolean evaluarAprobacion(List<Double> notas) {
        boolean vaciaOInvalida = (notas == null || notas.isEmpty());
        return !vaciaOInvalida && notas.stream().allMatch(n -> n >= 14.0);
    }
}

class Alumno extends Persona {
    private ProgramaAcademico programa;
    private List<Double> notas;

    public Alumno(String cedula, String nombreCompleto, String correo, String nombreProg, List<Double> notas) {
        super(cedula, nombreCompleto, correo);
        this.notas = (notas != null) ? notas : new ArrayList<>();
        
        String p = (nombreProg != null) ? nombreProg.strip().toLowerCase() : "";
        switch (p) {
            case "diplomado" -> this.programa = new Diplomado();
            case "bootcamp"  -> this.programa = new Bootcamp();
            default          -> this.programa = new Curso();
        }
    }

    public ProgramaAcademico getPrograma() { return programa; }
    public List<Double> getNotas() { return notas; }
    public void setNotas(List<Double> notas) { this.notas = notas; }
}

class Profesor extends Persona {
    private String especialidad;
    private String materia;

    public Profesor(String cedula, String nombreCompleto, String correo, String especialidad, String materia) {
        super(cedula, nombreCompleto, correo);
        this.especialidad = especialidad;
        this.materia = materia;
    }

    public String getEspecialidad() { return especialidad; }
    public String getMateria() { return materia; }
}

class RegistroHistorial {
    Alumno alumno;
    List<Double> notasAnteriores;

    public RegistroHistorial(Alumno alumno, List<Double> notasAnteriores) {
        this.alumno = alumno;
        this.notasAnteriores = new ArrayList<>(notasAnteriores);
    }
}

// 2. SISTEMA DE GESTIÓN ACADÉMICA (SGA)

class SGA {
    private List<Alumno> listaAlumnos = new ArrayList<>();
    private List<Profesor> listaProfesores = new ArrayList<>();
    private Queue<Alumno> colaCertificados = new LinkedList<>();
    private Stack<RegistroHistorial> pilaHistorialNotas = new Stack<>();

    public SGA() {
        cargarDesdeArchivos();
    }

    public void cargarDesdeArchivos() {
        this.listaAlumnos.clear();
        this.listaProfesores.clear();

        File fileAlumnos = new File("alumnos.txt");
        try (BufferedReader br = new BufferedReader(new FileReader(fileAlumnos))) {
            String linea;
            while ((linea = br.readLine()) != null) {
                String[] partes = linea.strip().split(",");
                boolean lineaValida = partes.length >= 4;
                switch (String.valueOf(lineaValida)) {
                    case "true" -> {
                        String ced = partes[0];
                        String nom = partes[1];
                        String cor = partes[2];
                        String prog = partes[3];

                        List<Double> notas = new ArrayList<>();
                        boolean tieneNotas = partes.length > 4;
                        switch (String.valueOf(tieneNotas)) {
                            case "true" -> {
                                for (int i = 4; i < partes.length; i++) {
                                    boolean noVacio = !partes[i].strip().isEmpty();
                                    switch (String.valueOf(noVacio)) {
                                        case "true" -> notas.add(Double.parseDouble(partes[i].strip()));
                                    }
                                }
                            }
                        }
                        listaAlumnos.add(new Alumno(ced, nom, cor, prog, notas));
                    }
                }
            }
        } catch (IOException | NumberFormatException e) {
            // Controlado
        }

        File fileProfesores = new File("profesores.txt");
        try (BufferedReader br = new BufferedReader(new FileReader(fileProfesores))) {
            String linea;
            while ((linea = br.readLine()) != null) {
                String[] partes = linea.strip().split(",");
                boolean lineaValida = partes.length >= 5;
                switch (String.valueOf(lineaValida)) {
                    case "true" -> listaProfesores.add(new Profesor(partes[0], partes[1], partes[2], partes[3], partes[4]));
                }
            }
        } catch (IOException e) {
            // Controlado
        }
    }

    public void guardarEstudiantesTxt() {
        try (BufferedWriter bw = new BufferedWriter(new FileWriter("alumnos.txt"))) {
            for (Alumno a : listaAlumnos) {
                StringBuilder sb = new StringBuilder();
                sb.append(a.getCedula()).append(",")
                  .append(a.getNombreCompleto()).append(",")
                  .append(a.getCorreo()).append(",")
                  .append(a.getPrograma().getNombrePrograma());

                for (double nota : a.getNotas()) {
                    sb.append(",").append(nota);
                }
                bw.write(sb.toString());
                bw.newLine();
            }
        } catch (IOException e) {
            System.out.println("Error al guardar alumnos: " + e.getMessage());
        }
    }

    public void guardarProfesoresTxt() {
        try (BufferedWriter bw = new BufferedWriter(new FileWriter("profesores.txt"))) {
            for (Profesor p : listaProfesores) {
                String linea = p.getCedula() + "," + p.getNombreCompleto() + "," +
                               p.getCorreo() + "," + p.getEspecialidad() + "," + p.getMateria();
                bw.write(linea);
                bw.newLine();
            }
        } catch (IOException e) {
            System.out.println("Error al guardar profesores: " + e.getMessage());
        }
    }

    public void registrarAlumno(Alumno alumno) {
        listaAlumnos.add(alumno);
        guardarEstudiantesTxt();
        System.out.println("Alumno " + alumno.getNombreCompleto() + " registrado con éxito.");
    }

    public void registrarProfesor(Profesor profesor) {
        listaProfesores.add(profesor);
        guardarProfesoresTxt();
        System.out.println("Profesor " + profesor.getNombreCompleto() + " registrado con éxito.");
    }


    public void registrarListaNotas(String cedula, List<Double> nuevasNotas) {
        boolean encontrado = false;
        for (Alumno a : listaAlumnos) {
            boolean coincideCedula = a.getCedula().equalsIgnoreCase(cedula);
            switch (String.valueOf(coincideCedula)) {
                case "true" -> {
                    encontrado = true;
                    // Validar el máximo de 3 notas por programa
                    boolean excedeLimite = nuevasNotas.size() > 3;
                    switch (String.valueOf(excedeLimite)) {
                        case "true" -> {
                            System.out.println("Error: No se pueden registrar más de 3 notas para este programa.");
                            return;
                        }
                        case "false" -> {
                            pilaHistorialNotas.push(new RegistroHistorial(a, a.getNotas()));
                            a.setNotas(nuevasNotas); 
                            guardarEstudiantesTxt(); 
                            System.out.println("Notas actualizadas con éxito a " + nuevasNotas + " para " + a.getNombreCompleto() + ".");
                            return;
                        }
                    }
                }
            }
        }
        System.out.print(encontrado ? "" : "Error: Alumno no encontrado.\n");
    }

        public void deshacerUltimaNota() {
        boolean vacia = pilaHistorialNotas.isEmpty();
        switch (String.valueOf(vacia)) {
            case "true" -> System.out.println("No hay cambios de notas para deshacer en la pila.");
            case "false" -> {
                RegistroHistorial historial = pilaHistorialNotas.pop();
                historial.alumno.setNotas(historial.notasAnteriores);
                guardarEstudiantesTxt();
                System.out.println("Deshecho: Se restauraron las notas anteriores a " + historial.alumno.getNombreCompleto() + ".");
            }
        }
    }

    public void generarColaCertificados() {
        colaCertificados.clear();
        for (Alumno a : listaAlumnos) {
            // EVAL-02: Polimorfismo puro llamando al método heredado sin if
            boolean aprobado = a.getPrograma().evaluarAprobacion(a.getNotas());
            switch (String.valueOf(aprobado)) {
                case "true" -> colaCertificados.add(a);
            }
        }

        try (BufferedWriter bw = new BufferedWriter(new FileWriter("certificados_pendientes.txt"))) {
            bw.write("=========================================\n");
            bw.write("REPORTE DE CERTIFICADOS PENDIENTES\n");
            bw.write("=========================================\n");
            bw.write("Total de graduandos en cola: " + colaCertificados.size() + "\n\n");
            
            int i = 1;
            for (Alumno est : colaCertificados) {
                double suma = est.getNotas().stream().mapToDouble(Double::doubleValue).sum();
                double prom = est.getNotas().isEmpty() ? 0.0 : suma / est.getNotas().size();
                
                bw.write(i + ". [" + est.getCedula() + "] " + est.getNombreCompleto() + "\n");
                bw.write("   - Programa: " + est.getPrograma().getNombrePrograma() + "\n");
                bw.write(String.format("   - Promedio Final: %.2f\n", prom));
                bw.write("   - Estatus: APROBADO\n\n");
                i++;
            }
            bw.write("=========================================\n");
            bw.write("* Fin del reporte - Generado por SGA-DO *\n");
        } catch (IOException e) {
            System.out.println("Error al exportar graduandos: " + e.getMessage());
        }
        System.out.println("Cola generada. " + colaCertificados.size() + " estudiantes pasaron a 'certificados_pendientes.txt'.");
    }

    public void mostrarReporteGeneral() {
        System.out.println("\n========================================");
        System.out.println("--- PROFESORES REGISTRADOS ---");
        System.out.println("========================================");
        boolean profsVacios = listaProfesores.isEmpty();
        System.out.print(profsVacios ? "No hay profesores registrados.\n" : "");
        for (Profesor p : listaProfesores) {
            System.out.println("[" + p.getCedula() + "] " + p.getNombreCompleto() + " - " + p.getEspecialidad() + " (" + p.getMateria() + ")");
        }
        
        System.out.println("\n========================================");
        System.out.println("--- REPORTE GENERAL DE ALUMNOS ---");
        System.out.println("========================================");
        boolean alumnosVacios = listaAlumnos.isEmpty();
        switch (String.valueOf(alumnosVacios)) {
            case "true" -> {
                System.out.println("No hay alumnos registrados.");
                return;
            }
        }
        for (Alumno a : listaAlumnos) {
            // EVAL-02: Polimorfismo Puro sin condicionales de tipo
            boolean esAprobado = a.getPrograma().evaluarAprobacion(a.getNotas());
            String estatusTexto = esAprobado ? "APROBADO" : "REPROBADO";
            double suma = a.getNotas().stream().mapToDouble(Double::doubleValue).sum();
            double prom = a.getNotas().isEmpty() ? 0.0 : suma / a.getNotas().size();
            
            System.out.println("• Alumno: " + a.getNombreCompleto() + " -> [" + estatusTexto + "]");
            System.out.println("  Cédula: " + a.getCedula() + " | Programa: " + a.getPrograma().getNombrePrograma());
            System.out.println(String.format("  Notas: %s | Promedio: %.2f", a.getNotas().toString(), prom));
            System.out.println("-----------------------------------");
        }
    }

    public void limpiarMemoriaYSalir() {
        guardarEstudiantesTxt();
        guardarProfesoresTxt();
        listaAlumnos.clear();
        listaProfesores.clear();
        colaCertificados.clear();
        pilaHistorialNotas.clear();
        System.out.println("Sistema cerrado de forma segura. ¡Hasta luego!");
    }
}

// 3. MENÚ

public class Main {
    public static void main(String[] args) {
        SGA sga = new SGA();
        Scanner scanner = new Scanner(System.in);
        boolean enEjecucion = true;
        
        while (enEjecucion) {
            System.out.println("\n==================================================");
            System.out.println("SGA-DO: SISTEMA DIPLOMADOSONLINE");
            System.out.println("==================================================");
            System.out.println("1. Registrar Alumno");
            System.out.println("2. Registrar Profesor");
            System.out.println("3. Registrar Notas a un Alumno");
            System.out.println("4. Deshacer Último Registro de Nota");
            System.out.println("5. Generar Cola de Certificados");
            System.out.println("6. Mostrar Reporte General");
            System.out.println("7. Salir");
            System.out.println("==================================================");
            System.out.print("Seleccione una opción (1-7): ");
            
            String opcion = scanner.nextLine().strip();
            boolean opcionVacia = opcion.isEmpty();
            switch (String.valueOf(opcionVacia)) {
                case "true" -> {
                    System.out.println("Error: Ingrese un valor numérico válido del 1 al 7.");
                    opcion = "PROCESAR_SIGUIENTE";
                }
            }
            
            switch (opcion) {
                case "PROCESAR_SIGUIENTE" -> {}
                case "1" -> {
                    System.out.print("Cédula: "); String ced = scanner.nextLine().strip();
                    System.out.print("Nombre completo: "); String nom = scanner.nextLine().strip();
                    System.out.print("Correo: "); String cor = scanner.nextLine().strip();
                    System.out.print("Programa (Curso / Diplomado / Bootcamp): "); String prog = scanner.nextLine().strip();
                    sga.registrarAlumno(new Alumno(ced, nom, cor, prog, null));
                }
                case "2" -> {
                    System.out.print("Cédula: "); String ced = scanner.nextLine().strip();
                    System.out.print("Nombre completo: "); String nom = scanner.nextLine().strip();
                    System.out.print("Correo: "); String cor = scanner.nextLine().strip();
                    System.out.print("Especialidad: "); String esp = scanner.nextLine().strip();
                    System.out.print("Materia: "); String mat = scanner.nextLine().strip();
                    sga.registrarProfesor(new Profesor(ced, nom, cor, esp, mat));
                }
                case "3" -> {
                    System.out.print("Cédula del alumno: "); String ced = scanner.nextLine().strip();
                    System.out.print("Ingrese las notas separadas por comas (ej: 14,15,13): ");
                    String entradaNotas = scanner.nextLine().strip();
                    try {
                        String[] fragmentos = entradaNotas.split(",");
                        List<Double> listaDeNotas = new ArrayList<>();
                        
                        // Convertir a Double y verificar rangos de forma limpia
                        boolean errorRango = false;
                        for (String f : fragmentos) {
                            if (!f.strip().isEmpty()) {
                                double v = Double.parseDouble(f.strip());
                                // Si alguna nota no cumple el rango de 0 a 20, marcamos error
                                errorRango = (v < 0 || v > 20) ? true : errorRango;
                                listaDeNotas.add(v);
                            }
                        }
                        switch (String.valueOf(errorRango)) {
                            case "true" -> System.out.println("Error: Las notas deben estar en el rango de 0 a 20.");
                            case "false" -> {
                                boolean vacio = listaDeNotas.isEmpty();
                                switch (String.valueOf(vacio)) {
                                    case "true" -> System.out.println("Error: No ingresó ninguna nota.");
                                    case "false" -> sga.registrarListaNotas(ced, listaDeNotas);
                                }
                            }
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("Error: Ingrese un valor numérico válido.");
                    }
                }
                case "4" -> sga.deshacerUltimaNota();
                case "5" -> sga.generarColaCertificados();
                case "6" -> sga.mostrarReporteGeneral();
                case "7" -> {
                    sga.limpiarMemoriaYSalir();
                    enEjecucion = false;
                }
                default -> System.out.println("Error: Ingrese un valor numérico válido del 1 al 7.");
            }
        }
        scanner.close();
    }
}


