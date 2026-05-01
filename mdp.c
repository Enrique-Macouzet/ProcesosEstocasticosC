/*
 * mdp.c - Herramienta MDP (terminal) - FES Acatlán, UNAM
 *
 * Incluye: Ingreso, Visualización, 5 métodos (con pasos detallados),
 *          Comparación mejorada, y pantalla de despedida animada.
 * Compilar: gcc -o mdp mdp.c -lm
 * Ejecutar: ./mdp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <unistd.h>

/* ---------- CONSTANTES ---------- */
#define MAX_ESTADOS      20
#define MAX_DECISIONES   10
#define MAX_NOMBRE       16
#define MAX_VARS         (MAX_ESTADOS * MAX_DECISIONES)
#define MAX_RESTRICCIONES (MAX_ESTADOS + 2)
#define MAX_POLITICAS    100000
#define MAX_ITER_SIMPLEX 200

/* ---------- ESTRUCTURAS ---------- */
typedef struct {
    int num_estados, num_decisiones, tipo;
    char estados[MAX_ESTADOS][MAX_NOMBRE], decisiones[MAX_DECISIONES][MAX_NOMBRE];
    double costos[MAX_ESTADOS][MAX_DECISIONES];
    double transiciones[MAX_DECISIONES][MAX_ESTADOS][MAX_ESTADOS];
    bool estados_afectados[MAX_DECISIONES][MAX_ESTADOS];
} ModeloMDP;

ModeloMDP m;

/* ---------- UTILIDADES ---------- */
void limpiar() { printf("\033[2J\033[H"); }
void pausar() { printf("\nPresiona ENTER para continuar..."); while(getchar()!='\n'); getchar(); }
double leer_numero() {
    char buf[32]; fgets(buf,32,stdin); buf[strcspn(buf,"\n")]=0;
    char *p = strchr(buf,'/');
    if(p){ *p=0; double n=atof(buf), d=atof(p+1); return fabs(d)<1e-9?0:n/d; }
    return atof(buf);
}

/* ---------- PORTADA ---------- */
void mostrar_portada() {
    limpiar();
    printf("\033[1;33m");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║     UNIVERSIDAD NACIONAL AUTÓNOMA DE MÉXICO           ║\n");
    printf("║     FACULTAD DE ESTUDIOS SUPERIORES ACATLÁN           ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("\033[0m\n");
    printf("\033[1;34m       HERRAMIENTA MDP - PROCESOS ESTOCÁSTICOS\033[0m\n\n");
    printf("  Profesora: Cuéllar Aguayo Ada Ruth\n  Integrantes: Hernández Pérez Victoria\n               Martínez Macouzet Enrique\n\n\n");
    printf("  Esta herramienta permite modelar y resolver\n");
    printf("  Procesos Markovianos de Decisión mediante\n");
    printf("  5 métodos de solución diferentes.\n");
    pausar();
}

/* ---------- INGRESO DE DATOS ---------- */
void ingresar_datos() {
    limpiar(); printf("\033[1;34m═══════ INGRESO DE DATOS ═══════\033[0m\n");
    printf("Tipo (0=Costos,1=Ganancias): "); scanf("%d",&m.tipo); while(getchar()!='\n');
    printf("Núm. estados: "); scanf("%d",&m.num_estados); while(getchar()!='\n');
    for(int i=0;i<m.num_estados;i++){ printf("Estado %d: ",i); fgets(m.estados[i],MAX_NOMBRE,stdin); m.estados[i][strcspn(m.estados[i],"\n")]=0; }
    printf("Núm. decisiones: "); scanf("%d",&m.num_decisiones); while(getchar()!='\n');
    for(int i=0;i<m.num_decisiones;i++){ printf("Decisión %d: ",i); fgets(m.decisiones[i],MAX_NOMBRE,stdin); m.decisiones[i][strcspn(m.decisiones[i],"\n")]=0; }
    for(int d=0;d<m.num_decisiones;d++){
        printf("\033[1;33m--- Decisión %s ---\033[0m\n",m.decisiones[d]);
        for(int s=0;s<m.num_estados;s++){ int af; printf("Afecta a %s? (1/0): ",m.estados[s]); scanf("%d",&af); while(getchar()!='\n'); m.estados_afectados[d][s]=(af==1); }
        printf("Costos:\n");
        for(int s=0;s<m.num_estados;s++) if(m.estados_afectados[d][s]){ printf("  %s: ",m.estados[s]); m.costos[s][d]=leer_numero(); }
        printf("Matriz de transición (fracciones OK):\n");
        for(int s=0;s<m.num_estados;s++){
            if(!m.estados_afectados[d][s]) continue;
            printf("  Desde %s:\n",m.estados[s]); double sum=0;
            for(int s2=0;s2<m.num_estados;s2++){ printf("    -> %s: ",m.estados[s2]); m.transiciones[d][s][s2]=leer_numero(); sum+=m.transiciones[d][s][s2]; }
            if(fabs(sum-1.0)>0.001) printf("\033[1;31m¡Suma %.4f!\033[0m\n",sum);
        }
    }
    printf("\033[1;32mModelo cargado.\033[0m\n"); pausar();
}

/* ---------- VISUALIZACIÓN ---------- */
void visualizar() {
    limpiar(); printf("\033[1;34m═══════ VISUALIZACIÓN ═══════\033[0m\n");
    printf("%-8s","Estado"); for(int d=0;d<m.num_decisiones;d++) printf("%10s",m.decisiones[d]); printf("\n");
    for(int s=0;s<m.num_estados;s++){ printf("%-8s",m.estados[s]); for(int d=0;d<m.num_decisiones;d++) printf("%10.4f",m.estados_afectados[d][s]?m.costos[s][d]:0.0); printf("\n"); }
    for(int d=0;d<m.num_decisiones;d++){
        printf("\nMATRIZ TRANS. Decisión %s\n",m.decisiones[d]);
        printf("%-8s","Desde"); for(int s2=0;s2<m.num_estados;s2++) printf("%10s",m.estados[s2]); printf("        Σ\n");
        for(int s=0;s<m.num_estados;s++){
            if(!m.estados_afectados[d][s]) continue;
            printf("%-8s",m.estados[s]); double su=0;
            for(int s2=0;s2<m.num_estados;s2++){ printf("%10.4f",m.transiciones[d][s][s2]); su+=m.transiciones[d][s][s2]; }
            printf("%10.4f\n",su);
        }
    }
    pausar();
}

/* ========== GENERACIÓN DE POLÍTICAS ========== */
int generar_politicas(int politicas[][MAX_ESTADOS]) {
    int max_op[MAX_ESTADOS]={0}, dec_por_estado[MAX_ESTADOS][MAX_DECISIONES];
    for(int i=0;i<m.num_estados;i++){ int k=0; for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]) dec_por_estado[i][k++]=d; max_op[i]=k; }
    int total=1; for(int i=0;i<m.num_estados;i++) total*=max_op[i];
    for(int p=0;p<total;p++){
        int temp=p;
        for(int i=m.num_estados-1;i>=0;i--){
            politicas[p][i]=dec_por_estado[i][temp % max_op[i]];
            temp/=max_op[i];
        }
    }
    return total;
}

int buscar_indice_politica(int pol[MAX_ESTADOS]) {
    int total = 1;
    for(int i=0;i<m.num_estados;i++){ int cnt=0; for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]) cnt++; total*=cnt; }
    int politicas[total][MAX_ESTADOS];
    generar_politicas(politicas);
    for(int p=0;p<total;p++){
        int igual=1;
        for(int i=0;i<m.num_estados;i++) if(politicas[p][i]!=pol[i]){ igual=0; break; }
        if(igual) return p+1;
    }
    return -1;
}

/* ========== GAUSS-JORDAN ========== */
void gauss_jordan(double a[MAX_ESTADOS+1][MAX_ESTADOS+2], int n, int mostrar, const char **nombres_cols) {
    if(mostrar) {
        printf("Matriz aumentada inicial:\n   ");
        for(int j=0;j<=n;j++) printf("%8s ", nombres_cols[j]);
        printf("\n");
        for(int i=0;i<n;i++){ printf("%2s ", nombres_cols[i]); for(int j=0;j<=n;j++) printf("%8.4f ",a[i][j]); printf("\n"); }
    }
    for(int col=0;col<n;col++){
        int max=col;
        for(int row=col+1;row<n;row++) if(fabs(a[row][col])>fabs(a[max][col])) max=row;
        if(max!=col) for(int j=0;j<=n;j++){ double t=a[col][j]; a[col][j]=a[max][j]; a[max][j]=t; }
        double piv=a[col][col];
        if(fabs(piv)<1e-12) return;
        for(int j=0;j<=n;j++) a[col][j]/=piv;
        for(int row=0;row<n;row++) if(row!=col){ double f=a[row][col]; for(int j=0;j<=n;j++) a[row][j]-=f*a[col][j]; }
        if(mostrar){
            printf("Paso col %d:\n   ",col);
            for(int j=0;j<=n;j++) printf("%8s ", nombres_cols[j]);
            printf("\n");
            for(int i=0;i<n;i++){ printf("%2s ", nombres_cols[i]); for(int j=0;j<=n;j++) printf("%8.4f ",a[i][j]); printf("\n"); }
        }
    }
}

/* ========== ENUMERACIÓN EXHAUSTIVA ========== */
void enumeracion_exhaustiva() {
    limpiar(); printf("\033[1;34m═══ ENUMERACIÓN EXHAUSTIVA ═══\033[0m\n");
    int n=m.num_estados, total;
    int politicas[MAX_POLITICAS][MAX_ESTADOS];
    total = generar_politicas(politicas);
    printf("Total de políticas: %d\n\n",total);
    double mejor_esp = (m.tipo==0)?1e30:-1e30;
    int mejor_idx = -1, mejor_pol[MAX_ESTADOS];
    for(int p=0;p<total;p++){
        int *pol=politicas[p];
        printf("\033[1;33mR%d = (",p+1);
        for(int i=0;i<n;i++) {
            printf("%s", m.decisiones[pol[i]]);
            if(i < n-1) printf(", ");
        }
        printf(")\033[0m\n");
        double P[MAX_ESTADOS][MAX_ESTADOS]={{0}}, c[MAX_ESTADOS]={0};
        for(int i=0;i<n;i++){ int d=pol[i]; c[i]=m.costos[i][d]; for(int j=0;j<n;j++) P[i][j]=m.transiciones[d][i][j]; }
        double A[MAX_ESTADOS+1][MAX_ESTADOS+2]={{0}};
        for(int j=0;j<n-1;j++){ for(int i=0;i<n;i++) A[j][i]=(i==j)?1.0-P[i][j]:-P[i][j]; A[j][n]=0.0; }
        for(int i=0;i<n;i++) A[n-1][i]=1.0; A[n-1][n]=1.0;
        printf("Sistema:\n");
        const char *nombres_gauss[MAX_ESTADOS+2];
        nombres_gauss[0]="π0"; for(int i=1;i<n;i++){ char tmp[16]; sprintf(tmp,"π%d",i); nombres_gauss[i]=strdup(tmp); }
        nombres_gauss[n]="RHS";
        gauss_jordan(A,n,1,nombres_gauss);
        double pi[MAX_ESTADOS]={0};
        for(int i=0;i<n;i++) pi[i]=A[i][n];
        double esp=0; for(int i=0;i<n;i++) esp+=pi[i]*c[i];
        printf("π: "); for(int i=0;i<n;i++) printf("%.4f ",pi[i]);
        printf(" | Costo esperado: %.4f\n",esp);
        if((m.tipo==0 && esp<mejor_esp) || (m.tipo==1 && esp>mejor_esp)){
            mejor_esp=esp; memcpy(mejor_pol,pol,n*sizeof(int)); mejor_idx = p+1;
        }
    }
    printf("\n\033[1;32mÓptima: R%d = (",mejor_idx);
    for(int i=0;i<n;i++) {
        printf("%s", m.decisiones[mejor_pol[i]]);
        if(i < n-1) printf(", ");
    }
    printf(") Costo: %.4f\033[0m\n",mejor_esp);
    pausar();
}

/* ========== SIMPLEX DOS FASES (con control verbose) ========== */
int simplex_dos_fases(double c_obj[MAX_VARS], double A_eq[MAX_RESTRICCIONES][MAX_VARS], double b_eq[MAX_RESTRICCIONES], int n_vars, int n_restr, double *x_opt, double *z_opt, const char **var_names, int verbose) {
    int n_art = n_restr;
    int n_total = n_vars + n_art;
    double tabla[MAX_RESTRICCIONES+1][MAX_VARS+MAX_RESTRICCIONES+1]={{0}};
    for(int i=0;i<n_restr;i++){
        for(int j=0;j<n_vars;j++) tabla[i][j] = A_eq[i][j];
        tabla[i][n_vars+i] = 1.0;
        tabla[i][n_total] = b_eq[i];
    }
    for(int j=0;j<n_vars;j++) tabla[n_restr][j] = 0.0;
    for(int j=n_vars;j<n_total;j++) tabla[n_restr][j] = 1.0;
    tabla[n_restr][n_total] = 0.0;
    for(int i=0;i<n_restr;i++) for(int j=0;j<=n_total;j++) tabla[n_restr][j] -= tabla[i][j];

    if(verbose) printf("\n--- Fase 1 ---\n");
    for(int iter=0;iter<MAX_ITER_SIMPLEX;iter++){
        int col_piv=-1; double min_val=0;
        for(int j=0;j<n_total;j++) if(tabla[n_restr][j] < min_val){ min_val=tabla[n_restr][j]; col_piv=j; }
        if(col_piv==-1) break;
        int fila_piv=-1; double razon=1e30;
        for(int i=0;i<n_restr;i++) if(tabla[i][col_piv]>1e-9){
            double r=tabla[i][n_total]/tabla[i][col_piv];
            if(r<razon){ razon=r; fila_piv=i; }
        }
        if(fila_piv==-1) return 0;
        double piv=tabla[fila_piv][col_piv];
        if(verbose) printf("Pivote: (%d,%d) = %.4f\n",fila_piv,col_piv,piv);
        for(int j=0;j<=n_total;j++) tabla[fila_piv][j]/=piv;
        for(int i=0;i<=n_restr;i++) if(i!=fila_piv){
            double f=tabla[i][col_piv];
            for(int j=0;j<=n_total;j++) tabla[i][j] -= f*tabla[fila_piv][j];
        }
        if(verbose) {
            for(int i=0;i<=n_restr;i++){ for(int j=0;j<=n_total;j++) printf("%8.4f ",tabla[i][j]); printf("\n"); }
            printf("\n");
        }
    }
    if(fabs(tabla[n_restr][n_total]) > 1e-6) return 0;
    if(verbose) printf("Fin fase 1.\n");

    for(int j=0;j<=n_total;j++) tabla[n_restr][j] = 0.0;
    for(int j=0;j<n_vars;j++) tabla[n_restr][j] = c_obj[j];
    for(int i=0;i<n_restr;i++){
        int var_basica = -1;
        for(int j=0;j<n_vars;j++) if(fabs(tabla[i][j]-1.0)<1e-9) var_basica=j;
        if(var_basica>=0){
            double coef = tabla[n_restr][var_basica];
            for(int j=0;j<=n_total;j++) tabla[n_restr][j] -= coef*tabla[i][j];
        }
    }

    if(verbose) printf("\n--- Fase 2 ---\n");
    for(int iter=0;iter<MAX_ITER_SIMPLEX;iter++){
        int col_piv=-1; double min_val=0;
        for(int j=0;j<n_vars;j++) if(tabla[n_restr][j] < min_val){ min_val=tabla[n_restr][j]; col_piv=j; }
        if(col_piv==-1) break;
        int fila_piv=-1; double razon=1e30;
        for(int i=0;i<n_restr;i++) if(tabla[i][col_piv]>1e-9){
            double r=tabla[i][n_total]/tabla[i][col_piv];
            if(r<razon){ razon=r; fila_piv=i; }
        }
        if(fila_piv==-1) return 0;
        double piv=tabla[fila_piv][col_piv];
        if(verbose) printf("Pivote: (%d,%d) = %.4f\n",fila_piv,col_piv,piv);
        for(int j=0;j<=n_total;j++) tabla[fila_piv][j]/=piv;
        for(int i=0;i<=n_restr;i++) if(i!=fila_piv){
            double f=tabla[i][col_piv];
            for(int j=0;j<=n_total;j++) tabla[i][j] -= f*tabla[fila_piv][j];
        }
        if(verbose) {
            for(int i=0;i<=n_restr;i++){ for(int j=0;j<=n_total;j++) printf("%8.4f ",tabla[i][j]); printf("\n"); }
            printf("\n");
        }
    }

    for(int j=0;j<n_vars;j++) x_opt[j]=0.0;
    for(int i=0;i<n_restr;i++){
        int var_basica=-1;
        for(int j=0;j<n_vars;j++) if(fabs(tabla[i][j]-1.0)<1e-9){ var_basica=j; break; }
        if(var_basica>=0) x_opt[var_basica]=tabla[i][n_total];
    }
    *z_opt = tabla[n_restr][n_total];
    return 1;
}

/* ========== PROGRAMACIÓN LINEAL ========== */
void programacion_lineal() {
    limpiar(); printf("\033[1;34m═══ PROGRAMACIÓN LINEAL ═══\033[0m\n");
    int nv=0, var_idx[MAX_ESTADOS][MAX_DECISIONES]; memset(var_idx,-1,sizeof(var_idx));
    char var_names[MAX_VARS][32];
    for(int i=0;i<m.num_estados;i++) for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
        var_idx[i][d]=nv; sprintf(var_names[nv],"Y_%s,%s",m.estados[i],m.decisiones[d]); nv++;
    }
    double c_obj[MAX_VARS]={0}, A_eq[MAX_RESTRICCIONES][MAX_VARS]={{0}}, b_eq[MAX_RESTRICCIONES]={0}, x_opt[MAX_VARS]={0}, z_opt;
    for(int i=0;i<m.num_estados;i++) for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0) c_obj[var_idx[i][d]]=(m.tipo==0)?m.costos[i][d]:-m.costos[i][d];
    int nr=0;
    for(int i=0;i<m.num_estados;i++) for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0) A_eq[nr][var_idx[i][d]]=1.0;
    b_eq[nr++]=1.0;
    for(int s=0;s<m.num_estados-1;s++){
        for(int i=0;i<m.num_estados;i++) for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0){
            if(i==s) A_eq[nr][var_idx[i][d]]+=1.0;
            A_eq[nr][var_idx[i][d]]-=m.transiciones[d][i][s];
        }
        b_eq[nr++]=0.0;
    }
    printf("\nFunción objetivo: Z = ");
    for(int i=0;i<nv;i++) if(c_obj[i]!=0) printf("%+.4f %s ",c_obj[i],var_names[i]); printf("\n");
    printf("Restricciones:\n");
    for(int i=0;i<nr;i++){
        int primero=1;
        for(int j=0;j<nv;j++) if(A_eq[i][j]!=0){
            printf("%+.4f %s ",A_eq[i][j],var_names[j]); primero=0;
        }
        printf("= %.4f\n",b_eq[i]);
    }
    printf("Y >= 0\n");
    const char *varnames_ptr[MAX_VARS];
    for(int i=0;i<nv;i++) varnames_ptr[i]=var_names[i];
    if(!simplex_dos_fases(c_obj,A_eq,b_eq,nv,nr,x_opt,&z_opt,varnames_ptr, 1)){ printf("No factible.\n"); pausar(); return; }
    printf("\nSolución óptima:\n");
    for(int i=0;i<m.num_estados;i++){ double sum=0; for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0) sum+=x_opt[var_idx[i][d]]; if(sum>1e-9){ for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0) printf("D(%s,%s)=%.4f ",m.estados[i],m.decisiones[d],x_opt[var_idx[i][d]]/sum); printf("\n"); } }
    double valor_final = (m.tipo==0) ? z_opt : -z_opt;
    printf("Valor óptimo: \033[1;32m%.4f\033[0m\n", fabs(valor_final));
    int pol_opt[MAX_ESTADOS];
    for(int i=0;i<m.num_estados;i++){ double maxD=-1; int best=-1; for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0){ double val=x_opt[var_idx[i][d]]; if(val>maxD){ maxD=val; best=d; } } pol_opt[i]=best; }
    int r_idx = buscar_indice_politica(pol_opt);
    printf("Política óptima: \033[1;34mR%d = (",r_idx);
    for(int i=0;i<m.num_estados;i++) {
        printf("%s", m.decisiones[pol_opt[i]]);
        if(i < m.num_estados-1) printf(", ");
    }
    printf(")\033[0m\n");
    pausar();
}

/* ========== MEJORAMIENTO DE POLÍTICAS (sin descuento) ========== */
void mejoramiento_politicas() {
    limpiar(); printf("\033[1;34m═══ MEJORAMIENTO DE POLÍTICAS (sin descuento) ═══\033[0m\n");
    int n=m.num_estados, total;
    int politicas_disp[MAX_POLITICAS][MAX_ESTADOS];
    total = generar_politicas(politicas_disp);
    printf("Políticas disponibles:\n");
    for(int p=0;p<total;p++){
        printf("R%d = (",p+1);
        for(int i=0;i<n;i++) {
            printf("%s", m.decisiones[politicas_disp[p][i]]);
            if(i < n-1) printf(", ");
        }
        printf(")\n");
    }
    int idx_inicial;
    printf("Elige política inicial (número R): "); scanf("%d",&idx_inicial); while(getchar()!='\n');
    int pol[MAX_ESTADOS]; memcpy(pol,politicas_disp[idx_inicial-1],n*sizeof(int));
    double V[MAX_ESTADOS]={0}, g;
    int iter=0;
    while(1){
        printf("\n--- Iteración %d ---\n",++iter);
        printf("Ecuaciones de evaluación (g + V_i - Σ P_ij V_j = C_i):\n");
        for(int i=0;i<n;i++){
            int d=pol[i];
            printf("g + V%d",i);
            int hay_terminos=0;
            for(int j=0;j<n;j++) if(m.transiciones[d][i][j]!=0) hay_terminos=1;
            if(hay_terminos){
                printf(" - (");
                int first=1;
                for(int j=0;j<n;j++) if(m.transiciones[d][i][j]!=0){
                    if(!first) printf(" + ");
                    printf("%.4f V%d",m.transiciones[d][i][j],j); first=0;
                }
                printf(")");
            }
            printf(" = %.4f\n",m.costos[i][d]);
        }
        double A[MAX_ESTADOS+1][MAX_ESTADOS+2]={{0}};
        for(int i=0;i<n;i++){
            int d=pol[i];
            A[i][0]=1.0;
            for(int j=0;j<n-1;j++) A[i][j+1] = -m.transiciones[d][i][j];
            A[i][i+1] += 1.0;
            A[i][n]=m.costos[i][d];
        }
        const char *nombres_g[MAX_ESTADOS+2];
        nombres_g[0]="g";
        for(int j=1;j<n;j++){ char tmp[16]; sprintf(tmp,"V%d",j-1); nombres_g[j]=tmp; }
        nombres_g[n]="RHS";
        printf("Sistema (g, V0..V%d):\n",n-2);
        gauss_jordan(A,n,1,nombres_g);
        g=A[0][n];
        for(int j=1;j<n;j++) V[j-1]=A[j][n];
        V[n-1]=0.0;
        printf("g=\033[1;32m%.4f\033[0m  V: ",g);
        for(int i=0;i<n;i++) printf("V%d=\033[1;33m%.4f\033[0m ",i,V[i]); printf("\n");

        // Paso 2: comparación de decisiones (mostrando C + Σ P V - V_i)
        printf("\nComparación de decisiones (C + Σ P V - V_i):\n");
        int nueva_pol[MAX_ESTADOS], igual=1;
        for(int i=0;i<n;i++){
            double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
            // Encontramos la mejor decisión (mínimo de C+ΣPV)
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+sum;
                if((m.tipo==0&&val<mejor)||(m.tipo==1&&val>mejor)){ mejor=val; best=d; }
            }
            nueva_pol[i]=best;
            if(best!=pol[i]) igual=0;
            // Ahora imprimimos tabla con C+ΣPV - V_i
            printf("Estado %s:\n", m.estados[i]);
            printf("  %-12s %-12s %s\n", "Decisión", "C+ΣPV - V_i", "Elegida");
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+sum;
                double diff = val - V[i];
                printf("  %-12s %-12.4f %s\n", m.decisiones[d], diff, (d==best)?"✅":"");
            }
        }
        int idx_nueva = buscar_indice_politica(nueva_pol);
        printf("Nueva política: R%d = (",idx_nueva);
        for(int i=0;i<n;i++) {
            printf("%s", m.decisiones[nueva_pol[i]]);
            if(i < n-1) printf(", ");
        }
        printf(")\n");
        if(igual){ printf("Política estable (R%d = R%d).\n", idx_nueva, idx_nueva); break; }
        memcpy(pol,nueva_pol,sizeof(pol));
        if(iter>100) break;
    }
    int idx_opt = buscar_indice_politica(pol);
    printf("\033[1;32mÓptima: R%d = (",idx_opt);
    for(int i=0;i<n;i++) {
        printf("%s", m.decisiones[pol[i]]);
        if(i < n-1) printf(", ");
    }
    printf(") g=%.4f\033[0m\n",g);
    pausar();
}

/* ========== MEJORAMIENTO CON DESCUENTO ========== */
void mejoramiento_descuento() {
    limpiar(); printf("\033[1;34m═══ MEJORAMIENTO CON DESCUENTO ═══\033[0m\n");
    int n=m.num_estados, total;
    int politicas_disp[MAX_POLITICAS][MAX_ESTADOS];
    total = generar_politicas(politicas_disp);
    printf("Políticas disponibles:\n");
    for(int p=0;p<total;p++){
        printf("R%d = (",p+1);
        for(int i=0;i<n;i++) {
            printf("%s", m.decisiones[politicas_disp[p][i]]);
            if(i < n-1) printf(", ");
        }
        printf(")\n");
    }
    int idx_inicial;
    printf("Elige política inicial (número R): "); scanf("%d",&idx_inicial); while(getchar()!='\n');
    int pol[MAX_ESTADOS]; memcpy(pol,politicas_disp[idx_inicial-1],n*sizeof(int));
    double alpha;
    printf("Factor α: "); alpha=leer_numero();
    double V[MAX_ESTADOS]={0};
    int iter=0;
    while(1){
        printf("\n--- Iteración %d ---\n",++iter);
        printf("Ecuaciones: V_i = C_i + α Σ_j P_ij V_j\n");
        for(int i=0;i<n;i++){
            int d=pol[i];
            printf("V%d = %.4f",i,m.costos[i][d]);
            int hay_terminos=0;
            for(int j=0;j<n;j++) if(m.transiciones[d][i][j]!=0) hay_terminos=1;
            if(hay_terminos){
                printf(" + %.4f * (",alpha);
                int first=1;
                for(int j=0;j<n;j++) if(m.transiciones[d][i][j]!=0){
                    if(!first) printf(" + ");
                    printf("%.4f V%d",m.transiciones[d][i][j],j); first=0;
                }
                printf(")");
            }
            printf("\n");
        }
        double A[MAX_ESTADOS+1][MAX_ESTADOS+2]={{0}};
        const char *nombres_v[MAX_ESTADOS+2];
        for(int i=0;i<n;i++){
            int d=pol[i];
            for(int j=0;j<n;j++) A[i][j] = (i==j)?1.0 - alpha*m.transiciones[d][i][j] : -alpha*m.transiciones[d][i][j];
            A[i][n] = m.costos[i][d];
            char tmp[16]; sprintf(tmp,"V%d",i); nombres_v[i]=strdup(tmp);
        }
        nombres_v[n]="RHS";
        printf("Sistema (I - αP)V = C:\n");
        gauss_jordan(A,n,1,nombres_v);
        for(int i=0;i<n;i++) V[i]=A[i][n];
        printf("V: ");
        for(int i=0;i<n;i++) printf("V%d=\033[1;33m%.4f\033[0m ",i,V[i]); printf("\n");

        printf("\nComparación de decisiones (Q = C + α Σ P V):\n");
        int nueva_pol[MAX_ESTADOS], igual=1;
        for(int i=0;i<n;i++){
            double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+alpha*sum;
                if((m.tipo==0&&val<mejor)||(m.tipo==1&&val>mejor)){ mejor=val; best=d; }
            }
            nueva_pol[i]=best;
            if(best!=pol[i]) igual=0;
            printf("Estado %s:\n", m.estados[i]);
            printf("  %-12s %-12s %s\n", "Decisión", "Q", "Elegida");
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+alpha*sum;
                printf("  %-12s %-12.4f %s\n", m.decisiones[d], val, (d==best)?"✅":"");
            }
        }
        int idx_nueva = buscar_indice_politica(nueva_pol);
        printf("Nueva política: R%d = (",idx_nueva);
        for(int i=0;i<n;i++) {
            printf("%s", m.decisiones[nueva_pol[i]]);
            if(i < n-1) printf(", ");
        }
        printf(")\n");
        if(igual){ printf("Política estable.\n"); break; }
        memcpy(pol,nueva_pol,sizeof(pol));
        if(iter>100) break;
    }
    int idx_opt = buscar_indice_politica(pol);
    printf("\033[1;32mÓptima: R%d = (",idx_opt);
    for(int i=0;i<n;i++) {
        printf("%s", m.decisiones[pol[i]]);
        if(i < n-1) printf(", ");
    }
    printf(")\033[0m\n");
    pausar();
}

/* ========== APROXIMACIONES SUCESIVAS ========== */
void aproximaciones_sucesivas() {
    limpiar(); printf("\033[1;34m═══ APROXIMACIONES SUCESIVAS ═══\033[0m\n");
    double V[MAX_ESTADOS], V_nuevo[MAX_ESTADOS], eps, alpha;
    int max_iter, pol[MAX_ESTADOS];
    printf("ε: "); scanf("%lf",&eps); while(getchar()!='\n');
    printf("Máx iter: "); scanf("%d",&max_iter); while(getchar()!='\n');
    printf("α: "); alpha=leer_numero();
    printf("\n--- Inicialización (V^1) ---\n");
    for(int i=0;i<m.num_estados;i++){
        double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
        for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
            if((m.tipo==0 && m.costos[i][d]<mejor) || (m.tipo==1 && m.costos[i][d]>mejor)){ mejor=m.costos[i][d]; best=d; }
        }
        V[i]=mejor; pol[i]=best;
        printf("V%d = %.4f (decisión %s)\n",i,V[i],m.decisiones[best]);
    }
    for(int it=2;it<=max_iter;it++){
        double max_dif=0;
        printf("\n--- Iteración %d ---\n",it);
        for(int i=0;i<m.num_estados;i++){
            double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<m.num_estados;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+alpha*sum;
                if((m.tipo==0 && val<mejor) || (m.tipo==1 && val>mejor)){ mejor=val; best=d; }
            }
            V_nuevo[i]=mejor; pol[i]=best;
            double dif=fabs(mejor-V[i]); if(dif>max_dif) max_dif=dif;
            printf("Estado %s:\n", m.estados[i]);
            printf("  %-12s %-12s %s\n", "Decisión", "Q = C+αΣPV", "Elegida");
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<m.num_estados;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+alpha*sum;
                printf("  %-12s %-12.4f %s\n", m.decisiones[d], val, (d==best)?"✅":"");
            }
            printf("  -> V%d = %.4f (anterior %.4f, dif %.4f)\n",i,V_nuevo[i],V[i],dif);
        }
        for(int i=0;i<m.num_estados;i++) V[i]=V_nuevo[i];
        printf("Diferencia máxima: %.6f\n",max_dif);
        if(max_dif<eps){ printf("Convergencia alcanzada.\n"); break; }
    }
    int idx_pol = buscar_indice_politica(pol);
    printf("\033[1;32mPolítica óptima: R%d = (",idx_pol);
    for(int i=0;i<m.num_estados;i++) {
        printf("%s", m.decisiones[pol[i]]);
        if(i < m.num_estados-1) printf(", ");
    }
    printf(")\033[0m\n");
    pausar();
}

/* ========== COMPARACIÓN DE MÉTODOS ========== */
void comparacion() {
    limpiar(); printf("\033[1;34m═══ COMPARACIÓN DE MÉTODOS ═══\033[0m\n\n");

    int n = m.num_estados;
    int total_ee;
    int politicas[MAX_POLITICAS][MAX_ESTADOS];
    total_ee = generar_politicas(politicas);

    double costos[5];
    int politicas_res[5][MAX_ESTADOS];
    int indices_res[5] = {0};
    char *nombres[5] = {"Enumeración Exhaustiva", "Programación Lineal",
                        "Mejoramiento Políticas", "Mejoramiento Descuento",
                        "Aprox. Sucesivas"};
    double V_finales[5][MAX_ESTADOS];

    // Enumeración
    double mejor_esp_ee = (m.tipo==0)?1e30:-1e30;
    int mejor_idx_ee;
    for(int p=0;p<total_ee;p++){
        int *pol = politicas[p];
        double P[MAX_ESTADOS][MAX_ESTADOS]={{0}}, c[MAX_ESTADOS]={0};
        for(int i=0;i<n;i++){ int d=pol[i]; c[i]=m.costos[i][d]; for(int j=0;j<n;j++) P[i][j]=m.transiciones[d][i][j]; }
        double A[MAX_ESTADOS+1][MAX_ESTADOS+2]={{0}};
        for(int j=0;j<n-1;j++){ for(int i=0;i<n;i++) A[j][i]=(i==j)?1.0-P[i][j]:-P[i][j]; A[j][n]=0.0; }
        for(int i=0;i<n;i++) A[n-1][i]=1.0; A[n-1][n]=1.0;
        gauss_jordan(A,n,0,NULL);
        double pi[MAX_ESTADOS]={0}; for(int i=0;i<n;i++) pi[i]=A[i][n];
        double esp=0; for(int i=0;i<n;i++) esp+=pi[i]*c[i];
        if((m.tipo==0 && esp<mejor_esp_ee) || (m.tipo==1 && esp>mejor_esp_ee)){
            mejor_esp_ee = esp; mejor_idx_ee = p+1;
            memcpy(politicas_res[0], pol, n*sizeof(int));
        }
    }
    costos[0] = mejor_esp_ee;
    indices_res[0] = mejor_idx_ee;

    // Programación Lineal (verbose=0)
    int nv=0, var_idx[MAX_ESTADOS][MAX_DECISIONES]; memset(var_idx,-1,sizeof(var_idx));
    for(int i=0;i<n;i++) for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){ var_idx[i][d]=nv++; }
    double c_obj[MAX_VARS]={0}, A_eq[MAX_RESTRICCIONES][MAX_VARS]={{0}}, b_eq[MAX_RESTRICCIONES]={0}, x_opt[MAX_VARS]={0}, z_opt;
    for(int i=0;i<n;i++) for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0) c_obj[var_idx[i][d]]=(m.tipo==0)?m.costos[i][d]:-m.costos[i][d];
    int nr=0;
    for(int i=0;i<n;i++) for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0) A_eq[nr][var_idx[i][d]]=1.0;
    b_eq[nr++]=1.0;
    for(int s=0;s<n-1;s++){
        for(int i=0;i<n;i++) for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0){
            if(i==s) A_eq[nr][var_idx[i][d]]+=1.0;
            A_eq[nr][var_idx[i][d]]-=m.transiciones[d][i][s];
        }
        b_eq[nr++]=0.0;
    }
    if(simplex_dos_fases(c_obj,A_eq,b_eq,nv,nr,x_opt,&z_opt,NULL, 0)){
        double val_pl = (m.tipo==0)?z_opt:-z_opt;
        costos[1] = fabs(val_pl);
        int pol_pl[MAX_ESTADOS];
        for(int i=0;i<n;i++){ double maxD=-1; int best=-1; for(int d=0;d<m.num_decisiones;d++) if(var_idx[i][d]>=0){ double val=x_opt[var_idx[i][d]]; if(val>maxD){ maxD=val; best=d; } } pol_pl[i]=best; }
        memcpy(politicas_res[1], pol_pl, n*sizeof(int));
        indices_res[1] = buscar_indice_politica(pol_pl);
    } else costos[1] = NAN;

    // Mejoramiento sin descuento
    int pol_mp[MAX_ESTADOS]; memcpy(pol_mp, politicas[0], n*sizeof(int));
    double V[MAX_ESTADOS]={0}, g;
    int iter=0;
    while(1){
        double A[MAX_ESTADOS+1][MAX_ESTADOS+2]={{0}};
        for(int i=0;i<n;i++){
            int d=pol_mp[i];
            A[i][0]=1.0;
            for(int j=0;j<n-1;j++) A[i][j+1] = -m.transiciones[d][i][j];
            A[i][i+1] += 1.0;
            A[i][n]=m.costos[i][d];
        }
        gauss_jordan(A,n,0,NULL);
        g=A[0][n]; for(int j=1;j<n;j++) V[j-1]=A[j][n]; V[n-1]=0.0;
        int nueva_pol[MAX_ESTADOS], igual=1;
        for(int i=0;i<n;i++){
            double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*V[j];
                double val=m.costos[i][d]+sum;
                if((m.tipo==0&&val<mejor)||(m.tipo==1&&val>mejor)){ mejor=val; best=d; }
            }
            nueva_pol[i]=best;
            if(best!=pol_mp[i]) igual=0;
        }
        if(igual) break;
        memcpy(pol_mp,nueva_pol,sizeof(pol_mp));
        if(++iter>100) break;
    }
    costos[2] = g;
    memcpy(politicas_res[2], pol_mp, n*sizeof(int));
    indices_res[2] = buscar_indice_politica(pol_mp);

    // Mejoramiento con descuento
    double alpha = 0.9;
    int pol_mpd[MAX_ESTADOS]; memcpy(pol_mpd, politicas[0], n*sizeof(int));
    double Vd[MAX_ESTADOS]={0};
    iter=0;
    while(1){
        double A[MAX_ESTADOS+1][MAX_ESTADOS+2]={{0}};
        for(int i=0;i<n;i++){
            int d=pol_mpd[i];
            for(int j=0;j<n;j++) A[i][j] = (i==j)?1.0 - alpha*m.transiciones[d][i][j] : -alpha*m.transiciones[d][i][j];
            A[i][n] = m.costos[i][d];
        }
        gauss_jordan(A,n,0,NULL);
        for(int i=0;i<n;i++) Vd[i]=A[i][n];
        int nueva_pol[MAX_ESTADOS], igual=1;
        for(int i=0;i<n;i++){
            double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*Vd[j];
                double val=m.costos[i][d]+alpha*sum;
                if((m.tipo==0&&val<mejor)||(m.tipo==1&&val>mejor)){ mejor=val; best=d; }
            }
            nueva_pol[i]=best;
            if(best!=pol_mpd[i]) igual=0;
        }
        if(igual) break;
        memcpy(pol_mpd,nueva_pol,sizeof(pol_mpd));
        if(++iter>100) break;
    }
    costos[3] = NAN;
    memcpy(politicas_res[3], pol_mpd, n*sizeof(int));
    indices_res[3] = buscar_indice_politica(pol_mpd);
    memcpy(V_finales[3], Vd, n*sizeof(double));

    // Aproximaciones sucesivas
    double V_as[MAX_ESTADOS], V_nuevo_as[MAX_ESTADOS];
    int pol_as[MAX_ESTADOS];
    for(int i=0;i<n;i++){
        double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
        for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
            if((m.tipo==0 && m.costos[i][d]<mejor) || (m.tipo==1 && m.costos[i][d]>mejor)){ mejor=m.costos[i][d]; best=d; }
        }
        V_as[i]=mejor; pol_as[i]=best;
    }
    for(int it=2;it<=100;it++){
        double max_dif=0;
        for(int i=0;i<n;i++){
            double mejor=(m.tipo==0)?1e30:-1e30; int best=-1;
            for(int d=0;d<m.num_decisiones;d++) if(m.estados_afectados[d][i]){
                double sum=0; for(int j=0;j<n;j++) sum+=m.transiciones[d][i][j]*V_as[j];
                double val=m.costos[i][d]+sum;
                if((m.tipo==0 && val<mejor) || (m.tipo==1 && val>mejor)){ mejor=val; best=d; }
            }
            V_nuevo_as[i]=mejor; pol_as[i]=best;
            double dif=fabs(mejor-V_as[i]); if(dif>max_dif) max_dif=dif;
        }
        for(int i=0;i<n;i++) V_as[i]=V_nuevo_as[i];
        if(max_dif<0.001) break;
    }
    costos[4] = NAN;
    memcpy(politicas_res[4], pol_as, n*sizeof(int));
    indices_res[4] = buscar_indice_politica(pol_as);
    memcpy(V_finales[4], V_as, n*sizeof(double));

    printf("\n\033[1;33m%-25s %-12s %-20s %-30s\033[0m\n", "Método", "Costo/Gan.", "Política", "V finales (si aplica)");
    printf("-------------------------------------------------------------------------------------------\n");
    for(int i=0;i<5;i++){
        printf("%-25s ", nombres[i]);
        if(isnan(costos[i])) printf("%-12s ", "—");
        else printf("%-12.4f ", costos[i]);
        printf("R%d = (", indices_res[i]);
        for(int j=0;j<n;j++) {
            printf("%s", m.decisiones[politicas_res[i][j]]);
            if(j < n-1) printf(", ");
        }
        printf(")");
        if(isnan(costos[i]) && i>=3) {
            printf("  V: ");
            for(int j=0;j<n;j++) printf("V%d=%.2f ", j, V_finales[i][j]);
        }
        printf("\n");
    }

    int iguales = 1;
    for(int i=1;i<5;i++){
        int coincide=1;
        for(int j=0;j<n;j++) if(politicas_res[0][j]!=politicas_res[i][j]){ coincide=0; break; }
        if(!coincide){ iguales=0; break; }
    }
    if(iguales) printf("\n\033[1;32mTodos los métodos coinciden en la misma política óptima.\033[0m\n");
    else printf("\n\033[1;31mLos métodos NO coinciden en la política óptima.\033[0m\n");
    pausar();
}

/* ========== PANTALLA DE DESPEDIDA ========== */
void despedida() {
    limpiar();
    printf("\033[1;33m");
    printf("  ╔══════════════════════════════════════════╗\n");
    printf("  ║       ¡MUCHAS GRACIAS POR USAR           ║\n");
    printf("  ║      LA HERRAMIENTA MDP - FES ACATLÁN    ║\n");
    printf("  ╚══════════════════════════════════════════╝\n");
    printf("\033[0m\n");
    for(int i=0;i<5;i++){
        printf("       🚀  🚀  🚀  🚀  🚀\n");
        fflush(stdout);
        usleep(150000);
        printf("\033[1A");
        printf("      🚀  🚀  🚀  🚀  🚀  🚀\n");
        fflush(stdout);
        usleep(150000);
        printf("\033[1A");
        printf("     🚀  🚀  🚀  🚀  🚀  🚀  🚀\n");
        fflush(stdout);
        usleep(150000);
        printf("\033[1A");
        printf("    🚀  🚀  🚀  🚀  🚀  🚀  🚀  🚀\n");
        fflush(stdout);
        usleep(150000);
        printf("\033[1A");
        printf("   🚀  🚀  🚀  🚀  🚀  🚀  🚀  🚀  🚀\n");
        fflush(stdout);
        usleep(150000);
        printf("\033[1A");
    }
    printf("\n\nPresiona ENTER para salir...");
    getchar();
}

/* ========== MENÚ PRINCIPAL ========== */
int main() {
    mostrar_portada();
    int op;
    do {
        limpiar(); printf("\033[1;33m╔══════════════ MENÚ PRINCIPAL ══════════════╗\033[0m\n");
        printf("\033[1;33m║\033[0m  1. Ingreso de Datos                       \033[1;33m║\033[0m\n");
        printf("\033[1;33m║\033[0m  2. Visualización de Datos                  \033[1;33m║\033[0m\n");
        printf("\033[1;33m║\033[0m  3. Métodos de Solución                     \033[1;33m║\033[0m\n");
        printf("\033[1;33m║\033[0m  4. Comparación de Métodos                  \033[1;33m║\033[0m\n");
        printf("\033[1;33m║\033[0m  5. Salir                                   \033[1;33m║\033[0m\n");
        printf("\033[1;33m╚═════════════════════════════════════════════╝\033[0m\n");
        printf("Opción: "); scanf("%d",&op); while(getchar()!='\n');
        switch(op){
            case 1: ingresar_datos(); break;
            case 2: visualizar(); break;
            case 3: {
                int op2;
                do {
                    limpiar(); printf("\033[1;34mMÉTODOS DE SOLUCIÓN\033[0m\n1.Enum.Exh.\n2.Prog.Lineal\n3.Mej.Políticas\n4.Mej.Descuento\n5.Aprox.Sucesivas\n6.Volver\nOpción: ");
                    scanf("%d",&op2); while(getchar()!='\n');
                    switch(op2){
                        case 1: enumeracion_exhaustiva(); break;
                        case 2: programacion_lineal(); break;
                        case 3: mejoramiento_politicas(); break;
                        case 4: mejoramiento_descuento(); break;
                        case 5: aproximaciones_sucesivas(); break;
                    }
                } while(op2!=6);
                break;
            }
            case 4: comparacion(); break;
            case 5: despedida(); return 0;
            default: limpiar(); printf("Opción no válida\n"); pausar();
        }
    } while(1);
    return 0;
}
