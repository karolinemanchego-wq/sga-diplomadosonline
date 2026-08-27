import os
# 1 JERARQUÍA DE CLASES 
class Persona:
    def __init__(self, cedula: str, nombre_completo: str, correo: str):
        self.cedula = cedula
        self.nombre_completo = nombre_completo
        self.correo = correo

class Estudiante(Persona):
    def __init__(self, cedula: str, nombre_completo: str, correo: str, programa: str, notas: list = None):
        super().__init__(cedula, nombre_completo, correo)
        self.programa = programa.strip().capitalize()
        self.notas = notas if notas is not None else []

class Profesor(Persona):
    def __init__(self, cedula: str, nombre_completo: str, correo: str, especialidad: str, materia: str):
        super().__init__(cedula, nombre_completo, correo)
        self.especialidad = especialidad
        self.materia = materia

class ProgramaAcademico:
    def __init__(self, nombre_programa: str, duracion: int, nota_minima: float):
        self.nombre_programa = nombre_programa
        self.duracion = duracion
        self.nota_minima = nota_minima

    def evaluar_aprobacion(self, notas: list) -> bool:
        raise NotImplementedError("Este método debe ser implementado por la subclase.")

class Curso(ProgramaAcademico):
    def __init__(self, nombre_programa: str = "Curso", duracion: int = 1, nota_minima: float = 10.0):
        super().__init__(nombre_programa, duracion, nota_minima)

    def evaluar_aprobacion(self, notas: list) -> bool:
        if not notas:
            return False
        promedio = sum(notas) / len(notas)
        return promedio >= self.nota_minima

class Diplomado(ProgramaAcademico):
    def __init__(self, nombre_programa: str = "Diplomado", duracion: int = 1, nota_minima: float = 14.0):
        super().__init__(nombre_programa, duracion, nota_minima)

    def evaluar_aprobacion(self, notas: list) -> bool:
        if not notas:
            return False
        promedio = sum(notas) / len(notas)
        return promedio >= self.nota_minima

class Bootcamp(ProgramaAcademico):
    def __init__(self, nombre_programa: str = "Bootcamp", duracion: int = 1, nota_minima: float = 14.0):
        super().__init__(nombre_programa, duracion, nota_minima)

    def evaluar_aprobacion(self, notas: list) -> bool:
        if not notas:
            return False
        return all(n >= 14.0 for n in notas)

# 2 SISTEMA DE GESTIÓN ACADÉMICA 
class SGA:
    def __init__(self):
        self.lista_estudiantes = []
        self.lista_profesores = []
        self.cola_certificados = []
        self.pila_historial_notas = []
        self.cargar_desde_archivos()

    def cargar_desde_archivos(self):
        """Carga datos asegurando limpiar memoria previa para evitar duplicados"""
        self.lista_estudiantes.clear() 
        self.lista_profesores.clear()   
        
        if os.path.exists("alumnos.txt"):
            with open("alumnos.txt", "r", encoding="utf-8") as f:
                for linea in f:
                    partes = linea.strip().split(",")
                    if len(partes) >= 4:
                        ced, nom, cor, prog = partes[0], partes[1], partes[2], partes[3]
                        
                        notas = []
                        if len(partes) > 4:
                            notas = [float(n) for n in partes[4:] if n.strip() != ""]
                        
                        self.lista_estudiantes.append(Estudiante(ced, nom, cor, prog, notas))

        if os.path.exists("profesores.txt"):
            with open("profesores.txt", "r", encoding="utf-8") as f:
                for linea in f:
                    partes = linea.strip().split(",")
                    if len(partes) >= 5:
                        self.lista_profesores.append(Profesor(partes[0], partes[1], partes[2], partes[3], partes[4]))

    def guardar_estudiantes_txt(self):
        """CORRECCIÓN: Guarda dinámicamente cualquier cantidad de notas sin romper"""
        with open("alumnos.txt", "w", encoding="utf-8") as f:
            for e in self.lista_estudiantes:
                notas_str = ",".join(str(n) for n in e.notas)
                if notas_str:
                    f.write(f"{e.cedula},{e.nombre_completo},{e.correo},{e.programa},{notas_str}\n")
                else:
                    f.write(f"{e.cedula},{e.nombre_completo},{e.correo},{e.programa}\n")

    def guardar_profesores_txt(self):
        with open("profesores.txt", "w", encoding="utf-8") as f:
            for p in self.lista_profesores:
                f.write(f"{p.cedula},{p.nombre_completo},{p.correo},{p.especialidad},{p.materia}\n")

    def registrar_alumno(self, estudiante: Estudiante):
        self.lista_estudiantes.append(estudiante)
        self.guardar_estudiantes_txt()
        print(f"Estudiante {estudiante.nombre_completo} registrado con éxito.")

    def registrar_profesor(self, profesor: Profesor):
        self.lista_profesores.append(profesor)
        self.guardar_profesores_txt()
        print(f"Profesor {profesor.nombre_completo} registrado con éxito.")

    def registrar_nota(self, cedula: str, notas_limpias: list):
        """Opción 3: Registro con copia exacta de historial contra cierres abruptos"""
        for e in self.lista_estudiantes:
            if e.cedula == cedula:
                try:
                    nuevas_notas = [float(n) for n in notas_limpias]
                except ValueError:
                    print("Error: Uno o varios valores ingresados no son números válidos.")
                    return
                
                notas_anteriores = list(e.notas) 
                e.notas = nuevas_notas
                self.pila_historial_notas.append((e, notas_anteriores))
                
                self.guardar_estudiantes_txt() 
                print(f"Notas actualizadas con éxito a {nuevas_notas} para {e.nombre_completo}.")
                return
        print("Error:Estudiante no encontrado.")

    def deshacer_ultima_nota(self):
        """Opción 4: Lógica LIFO completa restaurando estado previo de notas"""
        if not self.pila_historial_notas:
            print(" No hay cambios de notas para deshacer en la pila.")
            return

        estudiante, notas_anteriores = self.pila_historial_notas.pop()
        estudiante.notas = notas_anteriores
        self.guardar_estudiantes_txt()
        print(f"Deshecho: Se restauraron las notas anteriores {notas_anteriores} a {estudiante.nombre_completo}.")
    def generar_cola_certificados(self):
        """Opción 5: Procesa en orden FIFO y exporta a  los aprobados exactos"""
        self.cola_certificados.clear()
        evaluadores = {
            "Curso": Curso(),
            "Diplomado": Diplomado(),
            "Bootcamp": Bootcamp()
        }
        for e in self.lista_estudiantes:
            evaluador = evaluadores.get(e.programa, Curso())
            if evaluador.evaluar_aprobacion(e.notas):
                self.cola_certificados.append(e)
        # Exportar  archivo físico
        with open("certificados_pendientes.txt", "w", encoding="utf-8") as f:
            f.write("=========================================\n")
            f.write("REPORTE DE CERTIFICADOS PENDIENTES\n")
            f.write("=========================================\n")
            f.write(f"Total de graduandos en cola: {len(self.cola_certificados)}\n\n")
            for i, est in enumerate(self.cola_certificados, 1):
                prom = sum(est.notas) / len(est.notas) if est.notas else 0.0
                f.write(f"{i}. [{est.cedula}] {est.nombre_completo}\n")
                f.write(f"   - Programa: {est.programa}\n")
                f.write(f"   - Promedio Final: {prom:.2f}\n")
                f.write(f"   - Estatus: APROBADO\n\n")
            f.write("=========================================\n")
            f.write("* Fin del reporte - Generado por SGA-DO *\n")

        print(f"Cola generada. {len(self.cola_certificados)} estudiantes pasaron a 'certificados_pendientes.txt'.")
    def mostrar_reporte_general(self):
        """Opción 6: Imprime en pantalla detallando explícitamente nombre aprobado/reprobado"""
        print("\n" + "="*40)
        print("--- PROFESORES REGISTRADOS ---")
        print("="*40)
        if not self.lista_profesores:
            print("No hay profesores registrados.")
        for p in self.lista_profesores:
            print(f"[{p.cedula}] {p.nombre_completo} - {p.especialidad} ({p.materia})")

        print("\n" + "="*40)
        print("--- REPORTE GENERAL DE ALUMNOS ---")
        print("="*40)
        if not self.lista_estudiantes:
            print("No hay alumnos registrados.")
            return

        evaluadores = {
            "Curso": Curso(),
            "Diplomado": Diplomado(),
            "Bootcamp": Bootcamp()
        }
        for e in self.lista_estudiantes:
            evaluador = evaluadores.get(e.programa, Curso())
            es_aprobado = evaluador.evaluar_aprobacion(e.notas)
            #  diga  "Nombre Aprobado" o "Nombre Reprobado"
            estatus_texto = "APROBADO" if es_aprobado else "REPROBADO"
            prom = sum(e.notas) / len(e.notas) if e.notas else 0.0
            print(f"• Alumno: {e.nombre_completo} -> [{estatus_texto}]")
            print(f"  Cédula: {e.cedula} | Programa: {e.programa}")
            print("-" * 35)

    def salir_sistema(self):
        """Opción 7: Guarda cambios pendientes de seguridad, limpia memoria y cierra"""
        print(" Guardando bases de datos de respaldo...")
        self.guardar_estudiantes_txt()
        self.guardar_profesores_txt()
        print(" Limpiando memoria RAM del sistema...")
        self.lista_estudiantes.clear()
        self.lista_profesores.clear()
        self.cola_certificados.clear()
        self.pila_historial_notas.clear()
        print(" Sistema cerrado de forma segura. ¡Hasta luego!")
# 3 MENÚ
if __name__ == "__main__":
    sga = SGA()
    while True:
        print("\n" + "="*40)
        print("      SISTEMA DE GESTIÓN ACADÉMICA (SGA)")
        print("="*40)
        print("1. Registrar Alumno")
        print("2. Registrar Profesor")
        print("3. Registrar Notas a un Alumno")
        print("4. Deshacer Último Registro de Nota")
        print("5. Generar Cola de Certificados")
        print("6. Mostrar Reporte General")
        print("7. Salir")
        
        opcion = input("Seleccione una opción (1-7): ").strip()

        if opcion == "1":
            ced = input("Cédula: ").strip()
            nom = input("Nombre completo: ").strip()
            cor = input("Correo: ").strip()
            prog = input("Programa (Curso / Diplomado / Bootcamp): ").strip()
            sga.registrar_alumno(Estudiante(ced, nom, cor, prog))

        elif opcion == "2":
            ced = input("Cédula: ").strip()
            nom = input("Nombre completo: ").strip()
            cor = input("Correo: ").strip()
            esp = input("Especialidad: ").strip()
            mat = input("Materia: ").strip()
            sga.registrar_profesor(Profesor(ced, nom, cor, esp, mat))

        elif opcion == "3":
            ced = input("Cédula del alumno: ").strip()
            try:
                entrada_notas = input("Ingrese las notas separadas por comas (ej: 14,14,13): ")
                notas_limpias = [n.strip() for n in entrada_notas.split(",") if n.strip() != ""]
                if not notas_limpias:
                    print(" Error: No ingresó ninguna nota.")
                else:
                    sga.registrar_nota(ced, notas_limpias)
            except ValueError:
                print(" Error: Asegúrese de ingresar solo números separados por comas.")

        elif opcion == "4":
            sga.deshacer_ultima_nota()

        elif opcion == "5":
            sga.generar_cola_certificados()

        elif opcion == "6":
            sga.mostrar_reporte_general()

        elif opcion == "7":
            sga.salir_sistema()
            break
        else:
            print(" Opción inválida. Intente de nuevo.")
