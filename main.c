/*****************************************************************************************************************************************************
QUESTOES PARA O PROF
    O QUE DEVEMOS FAZER QUANDO O DENOMINADOR É ZERO?
        o que estamos a fazer de momento é dizer que 1/0 = DBL_MAX ou INF

    a particula as vezes consegue fugir da darea - O QUE PODEMOS FAZER?
        tentei limitar a velocidade mas nao parece resultar
            devemos fazer isso?
*****************************************************************************************************************************************************/


/*****************************************************************************************************************************************************
TODO
    criar temas
*****************************************************************************************************************************************************/


/*****************************************************************************************************************************************************
ERROS
    a particula as vezes consegue fugir da darea
*****************************************************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <math.h>

#include <gtk/gtk.h>
#include <cairo.h>

#define MAX_X 800
#define MAX_Y 800
#define MAX_VEL 3000
#define MAX_CARGA 100
#define MAX_MASSA 100
#define MIN_MASSA 0.1
#define MAX_INTENSIDADE_CAMPO_MAGNETICO 5
#define MAX_INTENSIDADE_CAMPO_ELETRICO 800

#define BARREIRA 10

#define GRAFICO_ESCALA_MIN 0.01
#define GRAFICO_ESCALA_MAX 10

#define NUM_ELEMENTOS_TRAJETORIA 400

#define DT_NORMAL 10e-3

#define K_E 10e3
#define INF 10e9

gdouble dt = 0;

GtkWidget *window;
FILE *data;

//////////////////////////////////////////// PERSONAL_MATH ////////////////////////////////////////////

//////////////////////// personal_math.h ////////////////////
#define QUADRADO(x) ((x)*(x))

typedef struct vetor
{
    double x, y, z;
} vetor;

//////////////////////// personal_math.c ////////////////////

vetor vetor_criar(double x, double y, double z)
{
    vetor v = {x, y, z};
    return v;
}

vetor vetor_somar(vetor v1, vetor v2)
{
    vetor v3 = {v1.x + v2.x,
                v1.y + v2.y,
                v1.z + v2.z};
    return v3;
}

vetor vetor_subtrair(vetor v1, vetor v2)
{
    vetor v3 = {v1.x - v2.x,
                v1.y - v2.y,
                v1.z - v2.z};
    return v3;
}

vetor vetor_escalar(double escalar, vetor v1)
{
    vetor v3 = {escalar * v1.x,
                escalar * v1.y,
                escalar * v1.z};
    return v3;
}

vetor vetor_externo(vetor v1, vetor v2)
{
    vetor v3 = {v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x};
    return v3;
}

double vetor_interno(vetor v1, vetor v2)
{
    return v1.x * v2.x  +  v1.y * v2.y  +  v1.z * v2.z;
}

double vetor_norma(vetor v)
{
    return sqrt( QUADRADO(v.x) + QUADRADO(v.y) + QUADRADO(v.z) );
}

vetor vetor_unitario(vetor v)
{
    double norma = vetor_norma(v);
    v = vetor_escalar( 1/norma, v);
    return v;
}

vetor vetor_unitario_AB(vetor A, vetor B)
{
    vetor v = vetor_subtrair(B, A);
    v = vetor_unitario(v);
    return v;
}

double vetor_distancia(vetor v1, vetor v2)
{
    vetor v3 =vetor_subtrair(v1, v2);
    return vetor_norma(v3);
}

double vetor_angulo_com_eixo_x(vetor v)
{
    return ( v.x>0 ? atan(v.y/v.x) : M_PI + atan(v.y/v.x));
}

//////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////

//////////////////////// functions.h ////////////////////

typedef struct estrutura_particula
{
    vetor r, r0, v, a, F;
    double carga, massa;
    double angulo_velocidade_inicial, intensidade_velocidade_inicial;
    double energia_potencial_eletrico, energia_potencial_magnetico, energia_cinetica;
}estrutura_particula;

typedef struct estrutura_campo_magnetico
{
    vetor B;
    int sentido; //sinal componete no eixo z
    double intensidade; //componente no eixo z
}estrutura_campo_magnetico;

typedef struct estrutura_campo_eletrico //se for uniforme nao existe origem; se nao for nao existe angulo
{
    vetor E;
    gboolean e_uniforme;
    vetor origem;
    double angulo, intensidade; //intensidade "e a carga da particula geradora"    
}estrutura_campo_eletrico;

typedef enum enum_tema{claro, escuro} enum_tema;

typedef struct estrutura_opcoes
{
    double escala,
            grafico_posicao_escala_x,
            grafico_posicao_escala_y,
            grafico_velocidade_escala_x,
            grafico_velocidade_escala_y,
            grafico_acelaracao_escala_x,
            grafico_acelaracao_escala_y,
            grafico_energia_escala_x,
            grafico_energia_escala_y;
    gboolean ver_opcoes,
            grafico_posicao_ver_x,
            grafico_posicao_ver_y,
            grafico_velocidade_ver_x,
            grafico_velocidade_ver_y,
            grafico_acelaracao_ver_x,
            grafico_acelaracao_ver_y,
            grafico_energia_ver_cinetica,
            grafico_energia_ver_potencial_eletrico,
            grafico_energia_ver_potencial_magnetico,
            ver_referencial, 
            ver_forca,
            ver_velocidade,
            ver_campo_eletrico,
            ver_campo_magnetico,
            ver_trajetoria,
            campo_eletrico_unicorme,

            abrir_grafico_posicao,
            abrir_grafico_velocidade,
            abrir_grafico_acelaracao,
            abrir_grafico_energia;
    enum_tema tema;
}estrutura_opcoes;

//////////////////////// functions.c ////////////////////

vetor get_window_size(void)
{
    GdkDisplay   *display;
    GdkMonitor   *monitor;
    GdkRectangle  geom;

    display = gdk_display_get_default ();
    monitor = gdk_display_get_primary_monitor (display);
    gdk_monitor_get_geometry (monitor, &geom);

    return vetor_criar(geom.width, geom.height, 0);
}

void provider_create_from_file(gchar *file_name)
{
    GtkCssProvider *provider ;
    GdkDisplay     *display  ;
    GdkScreen      *screen   ;

    provider = gtk_css_provider_new ();
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_path (provider, file_name, NULL);    
    g_object_unref (provider);
}

gboolean fechar_programa()
{
    printf("- A fechar -\n");
    fclose(data);
    printf("Ficheiro data.bin fechado com sucesso.\n");
    gtk_main_quit();
    printf("Gtk foi encerrado com sucesso.\n");
    return FALSE;
}

void arrow_to(cairo_t *cr, double x, double y, double angulo, double comprimento)
{
    static double alpha = M_PI / 12; //angulo entre a reta em si e cada um das retas da seta (?)
    static double mult = 0.2; //tamanho relatico das retas da seta

    cairo_move_to(cr, x, y); //origem do vetor

    double delta_x = cos(angulo) * comprimento;
    double delta_y = sin(angulo) * comprimento;
    double original_delta_x = delta_x;
    double original_delta_y = delta_y;

    cairo_rel_line_to(cr, delta_x, delta_y);
    cairo_stroke(cr);

    cairo_move_to(cr, x + original_delta_x, y + original_delta_y);
    delta_x = cos(M_PI + angulo + alpha) * mult * comprimento;
    delta_y = sin(M_PI + angulo + alpha) * mult * comprimento;
    cairo_rel_line_to(cr, delta_x, delta_y);

    cairo_move_to(cr, x + original_delta_x, y + original_delta_y);
    delta_x = cos(M_PI + angulo - alpha) * mult * comprimento;
    delta_y = sin(M_PI + angulo - alpha) * mult * comprimento;
    cairo_rel_line_to(cr, delta_x, delta_y);

    cairo_stroke(cr);
}

double normalizar_norma(double x, double max, double mult)
{
    return max * (exp(mult*x/max) - 1) / (exp(mult*x/max) + 1);
}

estrutura_particula criar_particula(vetor r0, double angulo_velocidade_inicial, double intensidade_velocidade_inicial, double carga, double massa)
{
    estrutura_particula particula;

    particula.r = r0;
    particula.r0 = r0;
    particula.carga = carga;
    particula.massa = massa;
    particula.angulo_velocidade_inicial = angulo_velocidade_inicial;
    particula.intensidade_velocidade_inicial = intensidade_velocidade_inicial;
    particula.v = vetor_criar(cos(angulo_velocidade_inicial), sin(angulo_velocidade_inicial), 0);
    particula.v = vetor_escalar(intensidade_velocidade_inicial, particula.v);
    particula.a = vetor_criar(0, 0, 0);
    particula.F = vetor_criar(0, 0, 0);
    particula.energia_cinetica = 0;
    particula.energia_potencial_eletrico = 0;
    particula.energia_potencial_magnetico = 0;
    
    return particula;
}

estrutura_campo_magnetico criar_campo_magnetico(int sentido, double intensidade)
{
    estrutura_campo_magnetico campo_magnetico;

    campo_magnetico.B = vetor_criar(0, 0, sentido*intensidade);
    campo_magnetico.sentido = sentido;
    campo_magnetico.intensidade = intensidade;

    return campo_magnetico;
}

estrutura_campo_eletrico criar_campo_eletrico(gboolean e_uniforme, vetor origem, double angulo, double intensidade)
{
    estrutura_campo_eletrico campo_eletrico;

    campo_eletrico.e_uniforme = e_uniforme;
    campo_eletrico.origem =  origem;
    campo_eletrico.angulo = angulo;
    campo_eletrico.intensidade = intensidade;

    return campo_eletrico;
}

estrutura_opcoes criar_opcoes(double escala,
                            double grafico_posicao_escala_x,
                            double grafico_posicao_escala_y,
                            double grafico_velocidade_escala_x,
                            double grafico_velocidade_escala_y,
                            double grafico_acelaracao_escala_x,
                            double grafico_acelaracao_escala_y,
                            double grafico_energia_escala_x,
                            double grafico_energia_escala_y,
                            gboolean ver_opcoes,
                            gboolean grafico_posicao_ver_x,
                            gboolean grafico_posicao_ver_y,
                            gboolean grafico_velocidade_ver_x,
                            gboolean grafico_velocidade_ver_y,
                            gboolean grafico_acelaracao_ver_x,
                            gboolean grafico_acelaracao_ver_y,
                            gboolean grafico_energia_ver_cinetica,
                            gboolean grafico_energia_ver_potencial_eletrico,
                            gboolean grafico_energia_ver_potencial_magnetico,
                            gboolean ver_referencial, 
                            gboolean ver_forca,
                            gboolean ver_velocidade,
                            gboolean ver_campo_eletrico,
                            gboolean ver_campo_magnetico,
                            gboolean ver_trajetoria,
                            gboolean campo_eletrico_unicorme,
                            gboolean abrir_grafico_posicao,
                            gboolean abrir_grafico_velocidade,
                            gboolean abrir_grafico_acelaracao,
                            gboolean abrir_grafico_energia,
                            enum_tema tema)
{
    estrutura_opcoes opcoes;

    opcoes.escala = escala;
    opcoes.grafico_posicao_escala_x = grafico_posicao_escala_x;
    opcoes.grafico_posicao_escala_y = grafico_posicao_escala_y;
    opcoes.grafico_velocidade_escala_x = grafico_velocidade_escala_x;
    opcoes.grafico_velocidade_escala_y = grafico_velocidade_escala_y;
    opcoes.grafico_acelaracao_escala_x = grafico_acelaracao_escala_x;
    opcoes.grafico_acelaracao_escala_y = grafico_acelaracao_escala_y;
    opcoes.grafico_energia_escala_x = grafico_energia_escala_x;
    opcoes.grafico_energia_escala_y = grafico_energia_escala_y;
    opcoes.grafico_energia_ver_cinetica = grafico_energia_ver_cinetica;
    opcoes.grafico_energia_ver_potencial_eletrico = grafico_energia_ver_potencial_eletrico;
    opcoes.grafico_energia_ver_potencial_magnetico = grafico_energia_ver_potencial_magnetico;
    opcoes.ver_opcoes = ver_opcoes;

    opcoes.grafico_posicao_ver_x = grafico_posicao_ver_x;
    opcoes.grafico_posicao_ver_y = grafico_posicao_ver_y;
    opcoes.grafico_velocidade_ver_x = grafico_velocidade_ver_x;
    opcoes.grafico_velocidade_ver_y = grafico_velocidade_ver_y;
    opcoes.grafico_acelaracao_ver_x = grafico_acelaracao_ver_x;
    opcoes.grafico_acelaracao_ver_y = grafico_acelaracao_ver_y;

    opcoes.ver_referencial = ver_referencial;
    opcoes.ver_forca = ver_forca;
    opcoes.ver_velocidade = ver_velocidade;
    opcoes.ver_campo_eletrico = ver_campo_eletrico;
    opcoes.ver_campo_magnetico = ver_campo_magnetico;
    opcoes.ver_trajetoria = ver_trajetoria;
    opcoes.campo_eletrico_unicorme = campo_eletrico_unicorme;
    opcoes.abrir_grafico_posicao = abrir_grafico_posicao;
    opcoes.abrir_grafico_velocidade = abrir_grafico_velocidade;
    opcoes.abrir_grafico_acelaracao = abrir_grafico_acelaracao;
    opcoes.abrir_grafico_energia = abrir_grafico_energia;
    opcoes.tema = tema;

    return opcoes;
}

estrutura_particula particula;
estrutura_particula trajetoria;
estrutura_campo_magnetico campo_magnetico;
estrutura_campo_eletrico campo_eletrico;
estrutura_opcoes opcoes;

gboolean fc_spin_button_x_posicao_inicial_particula(GtkWidget *w)
{
    particula.r0.x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_spin_button_y_posicao_inicial_particula(GtkWidget *w)
{
    particula.r0.y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_scale_angulo_velocidade_inicial_particula(GtkWidget *w)
{
    particula.angulo_velocidade_inicial =  gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_spin_button_intensidade_velocidade_inicial_particula(GtkWidget *w)
{
    particula.intensidade_velocidade_inicial =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_spin_button_carga_particula(GtkWidget *w)
{
    particula.carga =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_spin_button_massa_particula(GtkWidget *w)
{
    particula.massa =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_button_sentido_campo_magnetico(GtkWidget *w)
{
    if(campo_magnetico.sentido == 1)
        gtk_button_set_label(GTK_BUTTON(w), "Sentido a apontar para fora do ecrã");
    else
        gtk_button_set_label(GTK_BUTTON(w), "Sentido a apontar para dentro do ecrã");
    campo_magnetico.sentido *= -1;
    return FALSE;
}

gboolean fc_spin_button_intensidade_campo_magnetico(GtkWidget *w)
{
    campo_magnetico.intensidade =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_check_button_campo_eletrico_uniforme(GtkWidget *w)
{
    campo_eletrico.e_uniforme =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_spin_button_x_origem_campo_eletrico(GtkWidget *w)
{
    campo_eletrico.origem.x =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_spin_button_y_origem_campo_eletrico(GtkWidget *w)
{
    campo_eletrico.origem.y =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_scale_angulo_campo_eletrico(GtkWidget *w)
{
    campo_eletrico.angulo =  gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_spin_button_intensidade_campo_eletrico(GtkWidget *w)
{
    campo_eletrico.intensidade =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
    return FALSE;
}

gboolean fc_button_parar_tempo(GtkWidget *button_parar_tempo, GtkWidget *button_continuar_tempo)
{
    dt = 0;
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_widget_set_sensitive(button_parar_tempo, FALSE);
    gtk_widget_set_sensitive(button_continuar_tempo, TRUE);
    return FALSE;
}

gboolean fc_button_reiniciar(GtkWidget *w)
{
    particula.r = particula.r0;
    particula.v = vetor_criar(cos(particula.angulo_velocidade_inicial), sin(particula.angulo_velocidade_inicial), 0);
    particula.v = vetor_escalar(particula.intensidade_velocidade_inicial, particula.v);
    particula.a = vetor_criar(0, 0, 0);
    particula.F = vetor_criar(0, 0, 0);
    return FALSE;
}

gboolean fc_button_continuar_tempo(GtkWidget *button_continuar_tempo, GtkWidget *button_parar_tempo)
{
    dt = DT_NORMAL;
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_widget_set_sensitive(button_parar_tempo, TRUE);
    gtk_widget_set_sensitive(button_continuar_tempo, FALSE);
    return FALSE;
}

gboolean fc_scale_escala(GtkWidget *w)
{
    opcoes.escala = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_ver_opcoes(GtkWidget *w, GtkWidget *window_opcoes)
{
    gtk_widget_show_all(window_opcoes);
    return FALSE;
}

gboolean fc_abrir_grafico_posicao(GtkWidget *w, GtkWidget *window_grafico_posicao)
{
    gtk_widget_show_all(window_grafico_posicao);
    return FALSE;
}

gboolean fc_scale_grafico_posicao_escala_x(GtkWidget *w)
{
    opcoes.grafico_posicao_escala_x = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_scale_grafico_posicao_escala_y(GtkWidget *w)
{
    opcoes.grafico_posicao_escala_y = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_check_button_grafico_posicao_ver_x(GtkWidget *w)
{
    opcoes.grafico_posicao_ver_x = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_check_button_grafico_posicao_ver_y(GtkWidget *w)
{
    opcoes.grafico_posicao_ver_y = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_abrir_grafico_velocidade(GtkWidget *w, GtkWidget *window_grafico_velocidade)
{
    gtk_widget_show_all(window_grafico_velocidade);
    return FALSE;
}

gboolean fc_scale_grafico_velocidade_escala_x(GtkWidget *w)
{
    opcoes.grafico_velocidade_escala_x = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_scale_grafico_velocidade_escala_y(GtkWidget *w)
{
    opcoes.grafico_velocidade_escala_y = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_check_button_grafico_velocidade_ver_x(GtkWidget *w)
{
    opcoes.grafico_velocidade_ver_x = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_check_button_grafico_velocidade_ver_y(GtkWidget *w)
{
    opcoes.grafico_velocidade_ver_y = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_abrir_grafico_acelaracao(GtkWidget *w, GtkWidget *window_grafico_acelaracao)
{
    gtk_widget_show_all(window_grafico_acelaracao);
    return FALSE;
}

gboolean fc_scale_grafico_acelaracao_escala_x(GtkWidget *w)
{
    opcoes.grafico_acelaracao_escala_x = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_scale_grafico_acelaracao_escala_y(GtkWidget *w)
{
    opcoes.grafico_acelaracao_escala_y = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_check_button_grafico_acelaracao_ver_x(GtkWidget *w)
{
    opcoes.grafico_acelaracao_ver_x = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_check_button_grafico_acelaracao_ver_y(GtkWidget *w)
{
    opcoes.grafico_acelaracao_ver_y = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_abrir_grafico_energia(GtkWidget *w, GtkWidget *window_grafico_energia)
{
    gtk_widget_show_all(window_grafico_energia);
    return FALSE;
}

gboolean fc_scale_grafico_energia_escala_x(GtkWidget *w)
{
    opcoes.grafico_energia_escala_x = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_scale_grafico_energia_escala_y(GtkWidget *w)
{
    opcoes.grafico_energia_escala_y = gtk_range_get_value(GTK_RANGE(w));
    return FALSE;
}

gboolean fc_check_button_grafico_energia_ver_potencial_eletrico(GtkWidget *w)
{
    opcoes.grafico_energia_ver_potencial_eletrico = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_check_button_grafico_energia_ver_potencial_magnetico(GtkWidget *w)
{
    opcoes.grafico_energia_ver_potencial_magnetico = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_check_button_grafico_energia_ver_cinetica(GtkWidget *w)
{
    opcoes.grafico_energia_ver_cinetica = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    return FALSE;
}

gboolean fc_ver_referencial(GtkWidget *w)
{
    opcoes.ver_referencial =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
    return FALSE;
}

gboolean fc_ver_forca(GtkWidget *w)
{
    opcoes.ver_forca =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
    return FALSE;
}

gboolean fc_ver_velocidade(GtkWidget *w)
{
    opcoes.ver_velocidade =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
    return FALSE;
}

gboolean fc_ver_campo_magnetico(GtkWidget *w)
{
    opcoes.ver_campo_magnetico =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
    return FALSE;
}

gboolean fc_ver_campo_eletrico(GtkWidget *w)
{
    opcoes.ver_campo_eletrico =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
    return FALSE;
}

gboolean fc_ver_trajetoria(GtkWidget *w)
{
    opcoes.ver_trajetoria =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
    return FALSE;
}

//////////////////////////////////////////// DRAWING ////////////////////////////////////////////

gboolean on_draw_event(GtkWidget *darea, cairo_t *cr)
{

    if ( fseek(data, 0, SEEK_END) != 0 )
        printf("    ERR - erro ao tentar ir para o fim do ficheiro\n");
    if ( fwrite(&particula, sizeof(estrutura_particula), 1, data) != 1 )
        printf("    ERR - erro ao tentar escrever no fim do ficheiro\n");

    static gdouble darea_width = 0, darea_height = 0;
    darea_width = gtk_widget_get_allocated_width(darea);
    darea_height = gtk_widget_get_allocated_height(darea);   
    double temp;

    //transformar o plano tal que a origem esteja no centro da darea e que os vetores ex e ey sejam os convencionados
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, 1, 0, 0, -1, darea_width/2, darea_height/2);
    cairo_transform(cr, &matrix);

    //desenhar eixos
    if (opcoes.ver_referencial)
    {
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to (cr, -darea_width/2, 0.0);
        cairo_line_to (cr, darea_width/2, 0.0);
        cairo_move_to (cr, 0.0, -darea_height/2);
        cairo_line_to (cr, 0.0, darea_height/2);
        cairo_stroke (cr);
    }
    
    cairo_scale(cr, opcoes.escala, opcoes.escala);

    //desenhar trajetoria
    if (opcoes.ver_trajetoria)
    {
        cairo_set_source_rgba(cr, 1., 0., 0., 0.4);
        cairo_set_line_width (cr, 2.0);
        if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
        {   
            for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
            {
                if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                    cairo_line_to(cr, trajetoria.r.x, trajetoria.r.y);
            }
        }
        else
        {
            fseek(data, 0, SEEK_SET);
            fread(&trajetoria, sizeof(estrutura_particula), 1, data);

            for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
            {
                if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                    cairo_line_to(cr, trajetoria.r.x, trajetoria.r.y);
            }
        }
        cairo_stroke (cr);
    }

    if(opcoes.ver_velocidade)
    {
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1.5);
        arrow_to(cr, particula.r.x, particula.r.y, 
                vetor_angulo_com_eixo_x(particula.v), 
                normalizar_norma(vetor_norma(particula.v), 100, 0.5));
    }

    if(opcoes.ver_forca)
    {
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1.5);
        arrow_to(cr, particula.r.x, particula.r.y, 
                vetor_angulo_com_eixo_x(particula.F), 
                normalizar_norma(vetor_norma(particula.F), 100, 0.5));
    }
    
    //desenhar a particula
    cairo_set_source_rgb(cr, 0., 0.5, 1.);
    cairo_arc (cr, particula.r.x, particula.r.y, 10, 0., 2 * M_PI);
    cairo_fill (cr);

    //update campo magnetico
    campo_magnetico.B = vetor_criar(0, 0, campo_magnetico.sentido * campo_magnetico.intensidade);

    //update campo eletrico
    if (fabs(campo_eletrico.intensidade) < 0.01)
    {
        campo_eletrico.E = vetor_criar(0, 0, 0);
    }
    else if (campo_eletrico.e_uniforme)
    {
        campo_eletrico.E = vetor_criar(cos(campo_eletrico.angulo) * campo_eletrico.intensidade, 
                                       sin(campo_eletrico.angulo) * campo_eletrico.intensidade, 
                                       0);
    }
    else
    {
        if (vetor_distancia(particula.r, campo_eletrico.origem) != 0)
            temp = K_E * campo_eletrico.intensidade / QUADRADO( vetor_distancia(particula.r, campo_eletrico.origem) );
        else
            temp = INF;

        campo_eletrico.E = vetor_escalar(temp, vetor_unitario_AB(campo_eletrico.origem, particula.r));
    }
    

    //update particula
    particula.F = vetor_escalar(particula.carga, vetor_somar(campo_eletrico.E, vetor_externo(particula.v, campo_magnetico.B)) );
    if (particula.massa != 0)
        particula.a = vetor_escalar(1/particula.massa, particula.F); 
    else
        particula.a = vetor_escalar(INF, particula.F); 
    

    //colisao elastica com a parede
    if(particula.r.x < -darea_width/2 /opcoes.escala || particula.r.x > darea_width/2 /opcoes.escala)
        particula.v.x *= -1;
    
    if(particula.r.y < -darea_height/2 /opcoes.escala || particula.r.y > darea_height/2 /opcoes.escala)
        particula.v.y *= -1;

    particula.v = vetor_somar(particula.v, vetor_escalar(dt, particula.a));

    //maximar velocidade
    if(vetor_norma(particula.v) > MAX_VEL)
        particula.v = vetor_escalar(MAX_VEL, vetor_unitario(particula.v));
    
    particula.r = vetor_somar(particula.r, vetor_escalar(dt, particula.v));

    //update energia cinetica
    particula.energia_cinetica = 0.5 * particula.massa * QUADRADO(vetor_norma(particula.v));

    //update energia potencial eletrico
    particula.energia_potencial_eletrico = particula.carga * vetor_norma(campo_eletrico.E) * vetor_distancia(particula.r, campo_eletrico.origem);

    //update energia potencial magnetico
    particula.energia_potencial_magnetico = 10000 * vetor_norma(campo_magnetico.B); 

    return FALSE;
}

gboolean on_draw_event_grafico_posicao(GtkWidget *darea, cairo_t *cr)
{
    if (gtk_widget_get_visible(darea))
    {
        static gdouble darea_width = 0, darea_height = 0, mult_width = 0, mult_height = 0;
        darea_width = gtk_widget_get_allocated_width(darea);
        darea_height = gtk_widget_get_allocated_height(darea);
        mult_width = darea_width / NUM_ELEMENTOS_TRAJETORIA;
        mult_height = 1;

        //transformar o plano tal que a origem esteja no centro da darea à direita e que os vetores ex e ey sejam os convencionados
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, 1, 0, 0, -1, 0, darea_height/2);
        cairo_transform(cr, &matrix);

        //desenhar eixos
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to (cr, 0.0, 0.0);
        cairo_line_to (cr, darea_width, 0.0);
        cairo_move_to (cr, 0.0, -darea_height/2);
        cairo_line_to (cr, 0.0, darea_height/2);
        cairo_stroke (cr);

        //escalar eixos
        cairo_scale(cr, opcoes.grafico_posicao_escala_x, opcoes.grafico_posicao_escala_y);

        //desenhar grafico x
        if (opcoes.grafico_posicao_ver_x)
        {
            cairo_set_source_rgba(cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.r.x);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.r.x);
                }
            }
            cairo_stroke (cr);
        }

        //desenhar grafico y
        if (opcoes.grafico_posicao_ver_y)
        {
            cairo_set_source_rgba(cr, 0., 0., 1., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.r.y);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.r.y);
                }
            }
            cairo_stroke (cr);
        }
    }
}

gboolean on_draw_event_grafico_velocidade(GtkWidget *darea, cairo_t *cr)
{
    if (gtk_widget_get_visible(darea))
    {
        static gdouble darea_width = 0, darea_height = 0, mult_width = 0, mult_height = 0;
        darea_width = gtk_widget_get_allocated_width(darea);
        darea_height = gtk_widget_get_allocated_height(darea);
        mult_width = darea_width / NUM_ELEMENTOS_TRAJETORIA;
        mult_height = 0.2;

        //transformar o plano tal que a origem esteja no centro da darea à direita e que os vetores ex e ey sejam os convencionados
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, 1, 0, 0, -1, 0, darea_height/2);
        cairo_transform(cr, &matrix);

        //desenhar eixos
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to (cr, 0.0, 0.0);
        cairo_line_to (cr, darea_width, 0.0);
        cairo_move_to (cr, 0.0, -darea_height/2);
        cairo_line_to (cr, 0.0, darea_height/2);
        cairo_stroke (cr);

        //escalar eixos
        cairo_scale(cr, opcoes.grafico_velocidade_escala_x, opcoes.grafico_velocidade_escala_y);

        //desenhar grafico x
        if (opcoes.grafico_velocidade_ver_x)
        {
            cairo_set_source_rgba(cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.v.x);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.v.x);
                }
            }
            cairo_stroke (cr);
        }

        //desenhar grafico y
        if (opcoes.grafico_velocidade_ver_y)
        {
            cairo_set_source_rgba(cr, 0., 0., 1., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.v.y);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.v.y);
                }
            }
            cairo_stroke (cr);
        }
    }
}

gboolean on_draw_event_grafico_acelaracao(GtkWidget *darea, cairo_t *cr)
{
    if (gtk_widget_get_visible(darea))
    {
        static gdouble darea_width = 0, darea_height = 0, mult_width = 0, mult_height = 0;
        darea_width = gtk_widget_get_allocated_width(darea);
        darea_height = gtk_widget_get_allocated_height(darea);
        mult_width = darea_width / NUM_ELEMENTOS_TRAJETORIA;
        mult_height = 0.008;

        //transformar o plano tal que a origem esteja no centro da darea à direita e que os vetores ex e ey sejam os convencionados
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, 1, 0, 0, -1, 0, darea_height/2);
        cairo_transform(cr, &matrix);

        //desenhar eixos
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to (cr, 0.0, 0.0);
        cairo_line_to (cr, darea_width, 0.0);
        cairo_move_to (cr, 0.0, -darea_height/2);
        cairo_line_to (cr, 0.0, darea_height/2);
        cairo_stroke (cr);

        //escalar eixos
        cairo_scale(cr, opcoes.grafico_acelaracao_escala_x, opcoes.grafico_acelaracao_escala_y);

        //desenhar grafico x
        if (opcoes.grafico_acelaracao_ver_x)
        {
            cairo_set_source_rgba(cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.a.x);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.a.x);
                }
            }
            cairo_stroke (cr);
        }

        //desenhar grafico y
        if (opcoes.grafico_acelaracao_ver_y)
        {
            cairo_set_source_rgba(cr, 0., 0., 1., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.a.y);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height*trajetoria.a.y);
                }
            }
            cairo_stroke (cr);
        }
    }
}

gboolean on_draw_event_grafico_energia(GtkWidget *darea, cairo_t *cr)
{
   if (gtk_widget_get_visible(darea))
    {
        static gdouble darea_width = 0, darea_height = 0, mult_width = 0, 
        mult_height_cinetica = 0, mult_height_potencial_eletrico = 0, mult_height_potencial_magnetico = 0;
        darea_width = gtk_widget_get_allocated_width(darea);
        darea_height = gtk_widget_get_allocated_height(darea);
        mult_width = darea_width / NUM_ELEMENTOS_TRAJETORIA;
        mult_height_cinetica = 0.0001;
        mult_height_potencial_eletrico = 0.00005;
        mult_height_potencial_magnetico = 0.005;

        //transformar o plano tal que a origem esteja no centro da darea à direita e que os vetores ex e ey sejam os convencionados
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, 1, 0, 0, -1, 0, darea_height/2);
        cairo_transform(cr, &matrix);

        //desenhar eixos
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
        cairo_move_to (cr, 0.0, 0.0);
        cairo_line_to (cr, darea_width, 0.0);
        cairo_move_to (cr, 0.0, -darea_height/2);
        cairo_line_to (cr, 0.0, darea_height/2);
        cairo_stroke (cr);

        //escalar eixos
        cairo_scale(cr, opcoes.grafico_energia_escala_x, opcoes.grafico_energia_escala_y);

        //desenhar grafico energia cinetica
        if (opcoes.grafico_energia_ver_cinetica)
        {
            cairo_set_source_rgba(cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height_cinetica*trajetoria.energia_cinetica);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height_cinetica*trajetoria.energia_cinetica);
                }
            }
            cairo_stroke (cr);
        }

        //desenhar grafico energia potencial eletrico
        if (opcoes.grafico_energia_ver_potencial_eletrico)
        {
            cairo_set_source_rgba(cr, 0., 1., 0., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height_potencial_eletrico*trajetoria.energia_potencial_eletrico);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height_potencial_eletrico*trajetoria.energia_potencial_eletrico);
                }
            }
            cairo_stroke (cr);
        }

        //desenhar grafico energia potencial magnetico
        if (opcoes.grafico_energia_ver_potencial_magnetico)
        {
            cairo_set_source_rgba(cr, 0., 0., 1., 1);
            cairo_set_line_width (cr, 2.0);
            if ( fseek(data, - (long) sizeof(estrutura_particula) * NUM_ELEMENTOS_TRAJETORIA , SEEK_END) == 0) 
            {   
                for (int i = 0; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height_potencial_magnetico*trajetoria.energia_potencial_magnetico);
                }
            }
            else
            {
                fseek(data, 0, SEEK_SET);
                fread(&trajetoria, sizeof(estrutura_particula), 1, data);

                for (int i = 1; i < NUM_ELEMENTOS_TRAJETORIA; ++i)
                {
                    if (fread(&trajetoria, sizeof(estrutura_particula), 1, data) == 1)
                        cairo_line_to(cr, mult_width*i, mult_height_potencial_magnetico*trajetoria.energia_potencial_magnetico);
                }
            }
            cairo_stroke (cr);
        }

        
    }
}

gboolean time_handler (GtkWidget *widget)
{
    if ((!GTK_IS_WIDGET(widget)))
        return FALSE;

    gtk_widget_queue_draw(widget);

    return TRUE;
}

//////////////////////////////////////////// MAIN ////////////////////////////////////////////

int main(int argc, char **argv)
{
    GtkWidget *window_opcoes, *window_grafico_posicao, *window_grafico_velocidade, *window_grafico_acelaracao, *window_grafico_energia;
    setlocale(LC_ALL, "");

    GtkWidget *darea, *darea_grafico_posicao, *darea_grafico_velocidade, *darea_grafico_acelaracao, *darea_grafico_energia;
    data = fopen("data.bin", "wb+");

    gtk_init(&argc, &argv);

    provider_create_from_file ("themes/dark.css");
    vetor tamanho_tela = get_window_size();

    //iniciar
    particula = criar_particula(vetor_criar(100,50,0), 3*M_PI_2, 100, -10, 1);
    campo_magnetico = criar_campo_magnetico(1, 0);
    campo_eletrico = criar_campo_eletrico(FALSE, vetor_criar(0, 0, 0), 0, 50);
    opcoes = criar_opcoes(1, 1, 1, 1, 1, 1, 1, 1, 1, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, claro);

    gtk_window_set_default_icon_from_file("assets/icon.png", NULL);

    //criar janela
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Força de Lorentz");
    gtk_window_set_default_size(GTK_WINDOW(window), tamanho_tela.x/2,  tamanho_tela.y/2);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //criar janela opcoes
    window_opcoes = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_opcoes), "Força de Lorentz - Opções");
    gtk_window_set_position(GTK_WINDOW(window_opcoes), GTK_WIN_POS_CENTER);

    //criar janela grafico posicao
    window_grafico_posicao = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_grafico_posicao), "Força de Lorentz - Gráfico Posicao");
    gtk_window_set_default_size(GTK_WINDOW(window_grafico_posicao), tamanho_tela.x/2,  tamanho_tela.y/2);
    gtk_window_set_position(GTK_WINDOW(window_grafico_posicao), GTK_WIN_POS_CENTER);

    //criar janela grafico velocidade
    window_grafico_velocidade = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_grafico_velocidade), "Força de Lorentz - Gráfico Velocidade");
    gtk_window_set_default_size(GTK_WINDOW(window_grafico_velocidade), tamanho_tela.x/2,  tamanho_tela.y/2);
    gtk_window_set_position(GTK_WINDOW(window_grafico_velocidade), GTK_WIN_POS_CENTER);

    //criar janela grafico acelaracao
    window_grafico_acelaracao = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_grafico_acelaracao), "Força de Lorentz - Gráfico Acelaracao");
    gtk_window_set_default_size(GTK_WINDOW(window_grafico_acelaracao), tamanho_tela.x/2,  tamanho_tela.y/2);
    gtk_window_set_position(GTK_WINDOW(window_grafico_acelaracao), GTK_WIN_POS_CENTER);

    //criar janela grafico energia
    window_grafico_energia = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_grafico_energia), "Força de Lorentz - Gráfico Energia");
    gtk_window_set_default_size(GTK_WINDOW(window_grafico_energia), tamanho_tela.x/2,  tamanho_tela.y/2);
    gtk_window_set_position(GTK_WINDOW(window_grafico_energia), GTK_WIN_POS_CENTER);

    //conectar botao de fecho
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(fechar_programa), NULL);
    g_signal_connect(G_OBJECT(window_opcoes), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(window_grafico_posicao), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(window_grafico_velocidade), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(window_grafico_acelaracao), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(window_grafico_energia), "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    //vbox1
    GtkWidget *vbox1;
    vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox1);

    //hbox1
    GtkWidget *hbox1;
    hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 0);

    //frame drawing area
    GtkWidget *frame_drawing_area;
    frame_drawing_area = gtk_frame_new("Simulação");
    gtk_frame_set_label_align(GTK_FRAME(frame_drawing_area), 0.5, 1.);
    gtk_box_pack_start(GTK_BOX(hbox1), frame_drawing_area, TRUE, TRUE, 0);
    gtk_widget_set_margin_start(frame_drawing_area, 5);
    gtk_widget_set_margin_end(frame_drawing_area, 5);
    gtk_widget_set_margin_bottom(frame_drawing_area, 5);

    //drawing area 
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(frame_drawing_area), darea);

//////////////////////// OPCOES ////////////////////

    //box opcoes
    GtkWidget *box_opcoes;
    box_opcoes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window_opcoes), box_opcoes);

//////////////////////// VARIAVEIS ////////////////////

    //frame variaveis
    GtkWidget *frame_variaveis;
    frame_variaveis = gtk_frame_new("Variáveis");
    gtk_frame_set_label_align(GTK_FRAME(frame_variaveis), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_variaveis, 10);
    gtk_widget_set_margin_end(frame_variaveis, 10);
    gtk_box_pack_start(GTK_BOX(box_opcoes), frame_variaveis, FALSE, FALSE, 0);

    //box variaveis
    GtkWidget *box_variaveis;
    box_variaveis = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(frame_variaveis), box_variaveis);

//////////////////////// PARTICULA ////////////////////

    //frame particula
    GtkWidget *frame_particula;
    frame_particula = gtk_frame_new("Partícula");
    gtk_frame_set_label_align(GTK_FRAME(frame_particula), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_particula, 10);
    gtk_widget_set_margin_end(frame_particula, 10);
    gtk_widget_set_margin_bottom(frame_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_variaveis), frame_particula, TRUE, TRUE, 0);

    //box particula
    GtkWidget *box_particula;
    box_particula = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_particula), box_particula);

//////////////////////// POSICAO INICIAL ////////////////////

    //frame posicao inicial particula
    GtkWidget *frame_posicao_inicial_particula;
    frame_posicao_inicial_particula = gtk_frame_new("Posição inicial");
    gtk_frame_set_label_align(GTK_FRAME(frame_posicao_inicial_particula), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_posicao_inicial_particula, 10);
    gtk_widget_set_margin_end(frame_posicao_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_particula), frame_posicao_inicial_particula, TRUE, TRUE, 0);

    //box posicao inicial particula
    GtkWidget *box_posicao_inicial_particula;
    box_posicao_inicial_particula = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_posicao_inicial_particula), box_posicao_inicial_particula);

    //box x posicao inicial particula
    GtkWidget *box_x_posicao_inicial_particula;
    box_x_posicao_inicial_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_posicao_inicial_particula), box_x_posicao_inicial_particula, FALSE, FALSE, 5);

    //label x posicao inicial particula
    GtkWidget *label_x_posicao_inicial_particula;
    label_x_posicao_inicial_particula = gtk_label_new("x");
    gtk_widget_set_margin_start(label_x_posicao_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_x_posicao_inicial_particula), label_x_posicao_inicial_particula, FALSE, FALSE, 0);

    //spin_button x posicao inicial particula
    GtkWidget  *spin_button_x_posicao_inicial_particula;
    spin_button_x_posicao_inicial_particula = gtk_spin_button_new_with_range(-MAX_X, MAX_X, 0.01);
    gtk_box_pack_end(GTK_BOX(box_x_posicao_inicial_particula), spin_button_x_posicao_inicial_particula, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_x_posicao_inicial_particula, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_x_posicao_inicial_particula), particula.r.x);
    g_signal_connect(G_OBJECT(spin_button_x_posicao_inicial_particula), "changed", G_CALLBACK(fc_spin_button_x_posicao_inicial_particula), NULL);

    //box y posicao inicial particula
    GtkWidget *box_y_posicao_inicial_particula;
    box_y_posicao_inicial_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_posicao_inicial_particula), box_y_posicao_inicial_particula, FALSE, FALSE, 5);

    //label y posicao inicial particula
    GtkWidget *label_y_posicao_inicial_particula;
    label_y_posicao_inicial_particula = gtk_label_new("y");
    gtk_widget_set_margin_start(label_y_posicao_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_y_posicao_inicial_particula), label_y_posicao_inicial_particula, FALSE, FALSE, 0);

    //spin_button y posicao inicial particula
    GtkWidget  *spin_button_y_posicao_inicial_particula;
    spin_button_y_posicao_inicial_particula = gtk_spin_button_new_with_range(-MAX_Y, MAX_Y, 0.01);
    gtk_box_pack_end(GTK_BOX(box_y_posicao_inicial_particula), spin_button_y_posicao_inicial_particula, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_y_posicao_inicial_particula, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_y_posicao_inicial_particula), particula.r.y);
    g_signal_connect(G_OBJECT(spin_button_y_posicao_inicial_particula), "changed", G_CALLBACK(fc_spin_button_y_posicao_inicial_particula), NULL);

//////////////////////// VELOCIDADE INICIAL PARTICULA ////////////////////

    //frame velocidade inicial particula
    GtkWidget *frame_velocidade_inicial_particula;
    frame_velocidade_inicial_particula = gtk_frame_new("Velocidade inicial");
    gtk_frame_set_label_align(GTK_FRAME(frame_velocidade_inicial_particula), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_velocidade_inicial_particula, 10);
    gtk_widget_set_margin_end(frame_velocidade_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_particula), frame_velocidade_inicial_particula, TRUE, TRUE, 0);

    //box velocidade inicial particula
    GtkWidget *box_velocidade_inicial_particula;
    box_velocidade_inicial_particula = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_velocidade_inicial_particula), box_velocidade_inicial_particula);

    //box angulo velocidade inicial particula
    GtkWidget *box_angulo_velocidade_inicial_particula;
    box_angulo_velocidade_inicial_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_velocidade_inicial_particula), box_angulo_velocidade_inicial_particula, FALSE, FALSE, 5);

    //label angulo velocidade inicial particula
    GtkWidget *label_angulo_velocidade_inicial_particula;
    label_angulo_velocidade_inicial_particula = gtk_label_new("Ângulo");
    gtk_widget_set_margin_start(label_angulo_velocidade_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_angulo_velocidade_inicial_particula), label_angulo_velocidade_inicial_particula, FALSE, FALSE, 0);

    //scale angulo velocidade inicial particula
    GtkWidget  *scale_angulo_velocidade_inicial_particula;
    scale_angulo_velocidade_inicial_particula = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 2*M_PI, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_angulo_velocidade_inicial_particula), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_angulo_velocidade_inicial_particula), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_angulo_velocidade_inicial_particula, 20);
    gtk_box_pack_end(GTK_BOX(box_angulo_velocidade_inicial_particula), scale_angulo_velocidade_inicial_particula, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_angulo_velocidade_inicial_particula), particula.angulo_velocidade_inicial);
    g_signal_connect(G_OBJECT(scale_angulo_velocidade_inicial_particula), "value_changed", G_CALLBACK(fc_scale_angulo_velocidade_inicial_particula), NULL);

    //box intensidade velocidade inicial particula
    GtkWidget *box_intensidade_velocidade_inicial_particula;
    box_intensidade_velocidade_inicial_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_velocidade_inicial_particula), box_intensidade_velocidade_inicial_particula, FALSE, FALSE, 5);

    //label intensidade velocidade inicial particula
    GtkWidget *label_intensidade_velocidade_inicial_particula;
    label_intensidade_velocidade_inicial_particula = gtk_label_new("Intensidade ");
    gtk_widget_set_margin_start(label_intensidade_velocidade_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_intensidade_velocidade_inicial_particula), label_intensidade_velocidade_inicial_particula, FALSE, FALSE, 0);

    //spin_button intensidade velocidade inicial particula
    GtkWidget  *spin_button_intensidade_velocidade_inicial_particula;
    spin_button_intensidade_velocidade_inicial_particula = gtk_spin_button_new_with_range(0, MAX_VEL, 0.01);
    gtk_box_pack_end(GTK_BOX(box_intensidade_velocidade_inicial_particula), spin_button_intensidade_velocidade_inicial_particula, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_intensidade_velocidade_inicial_particula, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_intensidade_velocidade_inicial_particula), particula.intensidade_velocidade_inicial);
    g_signal_connect(G_OBJECT(spin_button_intensidade_velocidade_inicial_particula), "changed", G_CALLBACK(fc_spin_button_intensidade_velocidade_inicial_particula), NULL);

    //box carga particula
    GtkWidget *box_carga_particula;
    box_carga_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_particula), box_carga_particula, TRUE, TRUE, 5);

    //label carga particula
    GtkWidget *label_carga_particula;
    label_carga_particula = gtk_label_new("Carga");
    gtk_widget_set_margin_start(label_carga_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_carga_particula), label_carga_particula, FALSE, FALSE, 0);

    //spin_button carga particula
    GtkWidget  *spin_button_carga_particula;
    spin_button_carga_particula = gtk_spin_button_new_with_range(-MAX_CARGA, MAX_CARGA, 0.01);
    gtk_box_pack_end(GTK_BOX(box_carga_particula), spin_button_carga_particula, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_carga_particula, 20);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_carga_particula), particula.carga);
    g_signal_connect(G_OBJECT(spin_button_carga_particula), "changed", G_CALLBACK(fc_spin_button_carga_particula), NULL);

    //box massa particula
    GtkWidget *box_massa_particula;
    box_massa_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_particula), box_massa_particula, TRUE, TRUE, 5);

    //label massa particula
    GtkWidget *label_massa_particula;
    label_massa_particula = gtk_label_new("Massa");
    gtk_widget_set_margin_start(label_massa_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_massa_particula), label_massa_particula, FALSE, FALSE, 0);

    //spin_button massa particula
    GtkWidget  *spin_button_massa_particula;
    spin_button_massa_particula = gtk_spin_button_new_with_range(MIN_MASSA, MAX_MASSA, 0.01);
    gtk_box_pack_end(GTK_BOX(box_massa_particula), spin_button_massa_particula, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_massa_particula, 20);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_massa_particula), particula.massa);
    g_signal_connect(G_OBJECT(spin_button_massa_particula), "changed", G_CALLBACK(fc_spin_button_massa_particula), NULL);

//////////////////////// CAMPO MAGNETICO ////////////////////

    //box campos
    GtkWidget *box_campos;
    box_campos = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box_variaveis), box_campos, TRUE, TRUE, 0);

    //frame campo magnetico
    GtkWidget *frame_campo_magnetico;
    frame_campo_magnetico = gtk_frame_new("Campo magnético");
    gtk_frame_set_label_align(GTK_FRAME(frame_campo_magnetico), 0.1, 0.5);
    gtk_widget_set_margin_end(frame_campo_magnetico, 10);
    gtk_box_pack_start(GTK_BOX(box_campos), frame_campo_magnetico, FALSE, FALSE, 0);

    //box campo magnetico
    GtkWidget *box_campo_magnetico;
    box_campo_magnetico = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_campo_magnetico), box_campo_magnetico);

    //button sentido campo magnetico
    GtkWidget *button_sentido_campo_magnetico;
    button_sentido_campo_magnetico = gtk_button_new_with_label("Sentido a apontar para dentro do ecrã");
    gtk_widget_set_margin_start(button_sentido_campo_magnetico, 10);
    gtk_widget_set_margin_end(button_sentido_campo_magnetico, 10);
    gtk_box_pack_start(GTK_BOX(box_campo_magnetico), button_sentido_campo_magnetico, FALSE, FALSE, 5);
    gtk_widget_set_size_request(button_sentido_campo_magnetico, 300, 34);
    fc_button_sentido_campo_magnetico(button_sentido_campo_magnetico);
    fc_button_sentido_campo_magnetico(button_sentido_campo_magnetico);
    g_signal_connect(G_OBJECT(button_sentido_campo_magnetico), "pressed", G_CALLBACK(fc_button_sentido_campo_magnetico), NULL);

    //box intensidade campo magnetico
    GtkWidget *box_intensidade_campo_magnetico;
    box_intensidade_campo_magnetico = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_campo_magnetico), box_intensidade_campo_magnetico, FALSE, FALSE, 5);

    //label intensidade campo magnetico
    GtkWidget *label_intensidade_campo_magnetico;
    label_intensidade_campo_magnetico = gtk_label_new("Intensidade");
    gtk_widget_set_margin_start(label_intensidade_campo_magnetico, 10);
    gtk_box_pack_start(GTK_BOX(box_intensidade_campo_magnetico), label_intensidade_campo_magnetico, FALSE, FALSE, 0);

    //spin_button intensidade campo magnetico
    GtkWidget  *spin_button_intensidade_campo_magnetico;
    spin_button_intensidade_campo_magnetico = gtk_spin_button_new_with_range(0, MAX_INTENSIDADE_CAMPO_MAGNETICO, 0.01);
    gtk_box_pack_end(GTK_BOX(box_intensidade_campo_magnetico), spin_button_intensidade_campo_magnetico, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_intensidade_campo_magnetico, 20);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_intensidade_campo_magnetico), campo_magnetico.intensidade);
    g_signal_connect(G_OBJECT(spin_button_intensidade_campo_magnetico), "changed", G_CALLBACK(fc_spin_button_intensidade_campo_magnetico), NULL);

//////////////////////// CAMPO ELETRICO ////////////////////

    //frame campo eletrico
    GtkWidget *frame_campo_eletrico;
    frame_campo_eletrico = gtk_frame_new("Campo elétrico");
    gtk_frame_set_label_align(GTK_FRAME(frame_campo_eletrico), 0.1, 0.5);
    gtk_widget_set_margin_end(frame_campo_eletrico, 10);
    gtk_widget_set_margin_bottom(frame_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_campos), frame_campo_eletrico, FALSE, FALSE, 0);

    //box campo eletrico
    GtkWidget *box_campo_eletrico;
    box_campo_eletrico = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_campo_eletrico), box_campo_eletrico);

    //check button campo eletrico uniforme
    GtkWidget *check_button_campo_eletrico_uniforme;
    check_button_campo_eletrico_uniforme = gtk_check_button_new_with_label("Campo elétrico uniforme");
    gtk_widget_set_margin_start(check_button_campo_eletrico_uniforme, 10);
    gtk_widget_set_margin_end(check_button_campo_eletrico_uniforme, 10);
    gtk_box_pack_start(GTK_BOX(box_campo_eletrico), check_button_campo_eletrico_uniforme, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(check_button_campo_eletrico_uniforme), "toggled", G_CALLBACK(fc_check_button_campo_eletrico_uniforme), NULL);

    //frame origem campo eletrico
    GtkWidget *frame_origem_campo_eletrico;
    frame_origem_campo_eletrico = gtk_frame_new("Origem campo eletrico");
    gtk_frame_set_label_align(GTK_FRAME(frame_origem_campo_eletrico), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_origem_campo_eletrico, 10);
    gtk_widget_set_margin_end(frame_origem_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_campo_eletrico), frame_origem_campo_eletrico, FALSE, FALSE, 0);

    //box origem campo eletrico
    GtkWidget *box_origem_campo_eletrico;
    box_origem_campo_eletrico = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_origem_campo_eletrico), box_origem_campo_eletrico);


    //box x origem campo eletrico
    GtkWidget *box_x_origem_campo_eletrico;
    box_x_origem_campo_eletrico = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_origem_campo_eletrico), box_x_origem_campo_eletrico, FALSE, FALSE, 5);

    //label x origem campo eletrico
    GtkWidget *label_x_origem_campo_eletrico;
    label_x_origem_campo_eletrico = gtk_label_new("x de origem");
    gtk_widget_set_margin_start(label_x_origem_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_x_origem_campo_eletrico), label_x_origem_campo_eletrico, FALSE, FALSE, 0);

    //spin_button x origem campo eletrico
    GtkWidget  *spin_button_x_origem_campo_eletrico;
    spin_button_x_origem_campo_eletrico = gtk_spin_button_new_with_range(-MAX_X, MAX_X, 0.01);
    gtk_box_pack_end(GTK_BOX(box_x_origem_campo_eletrico), spin_button_x_origem_campo_eletrico, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_x_origem_campo_eletrico, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_x_origem_campo_eletrico), campo_eletrico.origem.x);
    g_signal_connect(G_OBJECT(spin_button_x_origem_campo_eletrico), "changed", G_CALLBACK(fc_spin_button_x_origem_campo_eletrico), NULL);

    //box y origem campo eletrico
    GtkWidget *box_y_origem_campo_eletrico;
    box_y_origem_campo_eletrico = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_origem_campo_eletrico), box_y_origem_campo_eletrico, FALSE, FALSE, 5);

    //label y origem campo eletrico
    GtkWidget *label_y_origem_campo_eletrico;
    label_y_origem_campo_eletrico = gtk_label_new("y de origem");
    gtk_widget_set_margin_start(label_y_origem_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_y_origem_campo_eletrico), label_y_origem_campo_eletrico, FALSE, FALSE, 0);

    //spin_button y origem campo eletrico
    GtkWidget  *spin_button_y_origem_campo_eletrico;
    spin_button_y_origem_campo_eletrico = gtk_spin_button_new_with_range(-MAX_Y, MAX_Y, 0.01);
    gtk_box_pack_end(GTK_BOX(box_y_origem_campo_eletrico), spin_button_y_origem_campo_eletrico, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_y_origem_campo_eletrico, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_y_origem_campo_eletrico), campo_eletrico.origem.y);
    g_signal_connect(G_OBJECT(spin_button_y_origem_campo_eletrico), "changed", G_CALLBACK(fc_spin_button_y_origem_campo_eletrico), NULL);

    //box angulo campo eletrico
    GtkWidget *box_angulo_campo_eletrico;
    box_angulo_campo_eletrico = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_campo_eletrico), box_angulo_campo_eletrico, FALSE, FALSE, 5);

    //label angulo campo eletrico
    GtkWidget *label_angulo_campo_eletrico;
    label_angulo_campo_eletrico = gtk_label_new("Ângulo");
    gtk_widget_set_margin_start(label_angulo_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_angulo_campo_eletrico), label_angulo_campo_eletrico, FALSE, FALSE, 0);

    //scale angulo campo eletrico
    GtkWidget  *scale_angulo_campo_eletrico;
    scale_angulo_campo_eletrico = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 2*M_PI, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_angulo_campo_eletrico), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_angulo_campo_eletrico), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_angulo_campo_eletrico, 20);
    gtk_box_pack_end(GTK_BOX(box_angulo_campo_eletrico), scale_angulo_campo_eletrico, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_angulo_campo_eletrico), campo_eletrico.angulo);
    g_signal_connect(G_OBJECT(scale_angulo_campo_eletrico), "value_changed", G_CALLBACK(fc_scale_angulo_campo_eletrico), NULL);

    //box intensidade campo eletrico
    GtkWidget *box_intensidade_campo_eletrico;
    box_intensidade_campo_eletrico = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_campo_eletrico), box_intensidade_campo_eletrico, FALSE, FALSE, 5);

    //label intensidade campo eletrico
    GtkWidget *label_intensidade_campo_eletrico;
    label_intensidade_campo_eletrico = gtk_label_new("Intensidade");
    gtk_widget_set_margin_start(label_intensidade_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_intensidade_campo_eletrico), label_intensidade_campo_eletrico, FALSE, FALSE, 0);

    //spin_button intensidade campo eletrico
    GtkWidget  *spin_button_intensidade_campo_eletrico;
    spin_button_intensidade_campo_eletrico = gtk_spin_button_new_with_range(-MAX_INTENSIDADE_CAMPO_ELETRICO, MAX_INTENSIDADE_CAMPO_ELETRICO, 0.01);
    gtk_box_pack_end(GTK_BOX(box_intensidade_campo_eletrico), spin_button_intensidade_campo_eletrico, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(spin_button_intensidade_campo_eletrico, 20);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_intensidade_campo_eletrico), campo_eletrico.intensidade);
    g_signal_connect(G_OBJECT(spin_button_intensidade_campo_eletrico), "changed", G_CALLBACK(fc_spin_button_intensidade_campo_eletrico), NULL);
    
    //frame controlo tempo
    GtkWidget *frame_controlo_tempo;
    frame_controlo_tempo = gtk_frame_new("Controlo do tempo");
    gtk_frame_set_label_align(GTK_FRAME(frame_controlo_tempo), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_controlo_tempo, 10);
    gtk_widget_set_margin_end(frame_controlo_tempo, 10);
    gtk_widget_set_margin_bottom(frame_controlo_tempo, 10);
    gtk_box_pack_start(GTK_BOX(box_opcoes), frame_controlo_tempo, FALSE, FALSE, 0);

    //box controlo tempo
    GtkWidget *box_controlo_tempo;
    box_controlo_tempo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_controlo_tempo), box_controlo_tempo);

    //button parar tempo
    GtkWidget *button_parar_tempo, *button_continuar_tempo;
    button_parar_tempo = gtk_button_new_with_label("Parar");
    gtk_box_pack_start(GTK_BOX(box_controlo_tempo), button_parar_tempo, TRUE, TRUE, 5);
    gtk_widget_set_margin_top(button_parar_tempo, 5);
    gtk_widget_set_margin_bottom(button_parar_tempo, 5);
    
    //button reiniciar simulacao
    GtkWidget *button_reiniciar;
    button_reiniciar = gtk_button_new_with_label("Reiniciar");
    gtk_box_pack_start(GTK_BOX(box_controlo_tempo), button_reiniciar, TRUE, TRUE, 5);
    gtk_widget_set_margin_top(button_reiniciar, 5);
    gtk_widget_set_margin_bottom(button_reiniciar, 5);
    g_signal_connect(G_OBJECT(button_reiniciar), "pressed", G_CALLBACK(fc_button_reiniciar), NULL);

    //button continuar tempo
    button_continuar_tempo = gtk_button_new_with_label("Continuar");
    gtk_box_pack_start(GTK_BOX(box_controlo_tempo), button_continuar_tempo, TRUE, TRUE, 5);
    gtk_widget_set_margin_top(button_continuar_tempo, 5);
    gtk_widget_set_margin_bottom(button_continuar_tempo, 5);

    g_signal_connect(G_OBJECT(button_parar_tempo), "pressed", G_CALLBACK(fc_button_parar_tempo), button_continuar_tempo);
    g_signal_connect(G_OBJECT(button_continuar_tempo), "pressed", G_CALLBACK(fc_button_continuar_tempo), button_parar_tempo);

    fc_button_parar_tempo(button_parar_tempo, button_continuar_tempo);

    //frame escala
    GtkWidget *frame_escala;
    frame_escala = gtk_frame_new("Zoom");
    gtk_frame_set_label_align(GTK_FRAME(frame_escala), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_escala, 10);
    gtk_widget_set_margin_end(frame_escala, 10);
    gtk_widget_set_margin_bottom(frame_escala, 10);
    gtk_box_pack_start(GTK_BOX(box_opcoes), frame_escala, FALSE, FALSE, 0);

    //box escala
    GtkWidget *box_escala;
    box_escala = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_escala), box_escala);

    //scale escala
    GtkWidget  *scale_escala;
    scale_escala = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.5, 2, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_escala), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_escala), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_escala, 20);
    gtk_box_pack_end(GTK_BOX(box_escala), scale_escala, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_escala), opcoes.escala);
    g_signal_connect(G_OBJECT(scale_escala), "value_changed", G_CALLBACK(fc_scale_escala), NULL);

//////////////////////// MENU BAR ////////////////////

    GtkWidget *menubar;
    menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox1), menubar, FALSE, FALSE, 0);

//////////////////////// VISUALIZACAO ////////////////////

    GtkWidget *item_visualizacao;
    item_visualizacao = gtk_menu_item_new_with_label("Vizualização");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_visualizacao);

    GtkWidget *menu_visualizacao;
    menu_visualizacao = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_visualizacao), menu_visualizacao);

    //ver opcoes
    GtkWidget *ver_opcoes;
    ver_opcoes = gtk_menu_item_new_with_label("Ver menu de opções");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_opcoes);
    g_signal_connect(G_OBJECT(ver_opcoes), "activate", G_CALLBACK(fc_ver_opcoes), window_opcoes);

    //ver referencial
    GtkWidget *ver_referencial;
    ver_referencial = gtk_check_menu_item_new_with_label("Ver referencial");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_referencial);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_referencial), opcoes.ver_referencial);
    g_signal_connect(G_OBJECT(ver_referencial), "activate", G_CALLBACK(fc_ver_referencial), NULL);

    //ver F
    GtkWidget *ver_forca;
    ver_forca = gtk_check_menu_item_new_with_label("Ver vetor força");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_forca);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_forca), opcoes.ver_forca);
    g_signal_connect(G_OBJECT(ver_forca), "activate", G_CALLBACK(fc_ver_forca), NULL);


    //ver v
    GtkWidget *ver_velocidade;
    ver_velocidade = gtk_check_menu_item_new_with_label("Ver vetor velocidade");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_velocidade);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_velocidade), opcoes.ver_velocidade);
    g_signal_connect(G_OBJECT(ver_velocidade), "activate", G_CALLBACK(fc_ver_velocidade), NULL);


    //ver B
    GtkWidget *ver_campo_magnetico;
    ver_campo_magnetico = gtk_check_menu_item_new_with_label("Ver vetor campo magnético");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_campo_magnetico);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_campo_magnetico), opcoes.ver_campo_magnetico);
    g_signal_connect(G_OBJECT(ver_campo_magnetico), "activate", G_CALLBACK(fc_ver_campo_magnetico), NULL);

    //ver E
    GtkWidget *ver_campo_eletrico;
    ver_campo_eletrico = gtk_check_menu_item_new_with_label("Ver vetor campo elétrico");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_campo_eletrico);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_campo_eletrico), opcoes.ver_campo_eletrico);
    g_signal_connect(G_OBJECT(ver_campo_eletrico), "activate", G_CALLBACK(fc_ver_campo_eletrico), NULL);

    //separador
    GtkWidget *separador1;
    separador1 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), separador1);

    //ver trajetoria
    GtkWidget *ver_trajetoria;
    ver_trajetoria = gtk_check_menu_item_new_with_label("Ver trajetória");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_trajetoria);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_trajetoria), opcoes.ver_trajetoria);
    g_signal_connect(G_OBJECT(ver_trajetoria), "activate", G_CALLBACK(fc_ver_trajetoria), NULL);

//////////////////////// GRAFICOS ////////////////////

    GtkWidget *item_abrir_graficos;
    item_abrir_graficos = gtk_menu_item_new_with_label("Abrir gráficos");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_abrir_graficos);

    GtkWidget *menu_abrir_graficos;
    menu_abrir_graficos = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_abrir_graficos), menu_abrir_graficos);

    //abrir grafico posicao
    GtkWidget *abrir_grafico_posicao;
    abrir_grafico_posicao = gtk_menu_item_new_with_label("Abrir gráfico r(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_posicao);
    g_signal_connect(G_OBJECT(abrir_grafico_posicao), "activate", G_CALLBACK(fc_abrir_grafico_posicao), window_grafico_posicao);

        //hbox grafico posicao
    GtkWidget *hbox_grafico_posicao;
    hbox_grafico_posicao = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window_grafico_posicao), hbox_grafico_posicao);

        //drawing area grafico posicao
    darea_grafico_posicao = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(hbox_grafico_posicao), darea_grafico_posicao, TRUE, TRUE, 0);

        //frame grafico posicao opcoes
    GtkWidget *frame_grafico_posicao_opcoes;
    frame_grafico_posicao_opcoes = gtk_frame_new("Opcões");
    gtk_frame_set_label_align(GTK_FRAME(frame_grafico_posicao_opcoes), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_grafico_posicao_opcoes, 10);
    gtk_widget_set_margin_end(frame_grafico_posicao_opcoes, 10);
    gtk_box_pack_start(GTK_BOX(hbox_grafico_posicao), frame_grafico_posicao_opcoes, FALSE, FALSE, 0);
 
        //box grafico posicao opcoes
    GtkWidget *box_grafico_posicao_opcoes;
    box_grafico_posicao_opcoes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(box_grafico_posicao_opcoes, 200, -1);
    gtk_container_add(GTK_CONTAINER(frame_grafico_posicao_opcoes), box_grafico_posicao_opcoes);

        //box grafico posicao escala x
    GtkWidget *box_grafico_posicao_escala_x;
    box_grafico_posicao_escala_x = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_posicao_opcoes), box_grafico_posicao_escala_x, FALSE, FALSE, 5);

        //label grafico posicao escala x
    GtkWidget *label_grafico_posicao_escala_x;
    label_grafico_posicao_escala_x = gtk_label_new("Escala x");
    gtk_widget_set_margin_start(label_grafico_posicao_escala_x, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_posicao_escala_x), label_grafico_posicao_escala_x, FALSE, FALSE, 0);

        //scale grafico posicao escala x
    GtkWidget  *scale_grafico_posicao_escala_x;
    scale_grafico_posicao_escala_x = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_posicao_escala_x), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_posicao_escala_x), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_posicao_escala_x, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_posicao_escala_x), scale_grafico_posicao_escala_x, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_posicao_escala_x), opcoes.grafico_posicao_escala_x);
    g_signal_connect(G_OBJECT(scale_grafico_posicao_escala_x), "value_changed", G_CALLBACK(fc_scale_grafico_posicao_escala_x), NULL);

        //box grafico posicao escala y
    GtkWidget *box_grafico_posicao_escala_y;
    box_grafico_posicao_escala_y = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_posicao_opcoes), box_grafico_posicao_escala_y, FALSE, FALSE, 5);

        //label grafico posicao escala y
    GtkWidget *label_grafico_posicao_escala_y;
    label_grafico_posicao_escala_y = gtk_label_new("Escala y");
    gtk_widget_set_margin_start(label_grafico_posicao_escala_y, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_posicao_escala_y), label_grafico_posicao_escala_y, FALSE, FALSE, 0);

        //scale grafico posicao escala y
    GtkWidget  *scale_grafico_posicao_escala_y;
    scale_grafico_posicao_escala_y = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_posicao_escala_y), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_posicao_escala_y), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_posicao_escala_y, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_posicao_escala_y), scale_grafico_posicao_escala_y, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_posicao_escala_y), opcoes.grafico_posicao_escala_y);
    g_signal_connect(G_OBJECT(scale_grafico_posicao_escala_y), "value_changed", G_CALLBACK(fc_scale_grafico_posicao_escala_y), NULL);

        //check button grafico posicao ver x
    GtkWidget *check_button_grafico_posicao_ver_x;
    check_button_grafico_posicao_ver_x = gtk_check_button_new_with_label("Ver gráfico da componente de x");
    gtk_box_pack_start(GTK_BOX(box_grafico_posicao_opcoes), check_button_grafico_posicao_ver_x, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_posicao_ver_x), opcoes.grafico_posicao_ver_x);
    g_signal_connect(G_OBJECT(check_button_grafico_posicao_ver_x), "toggled", G_CALLBACK(fc_check_button_grafico_posicao_ver_x), NULL);

        //check button grafico posicao ver y
    GtkWidget *check_button_grafico_posicao_ver_y;
    check_button_grafico_posicao_ver_y = gtk_check_button_new_with_label("Ver gráfico da componente de y");
    gtk_box_pack_start(GTK_BOX(box_grafico_posicao_opcoes), check_button_grafico_posicao_ver_y, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_posicao_ver_y), opcoes.grafico_posicao_ver_y);
    g_signal_connect(G_OBJECT(check_button_grafico_posicao_ver_y), "toggled", G_CALLBACK(fc_check_button_grafico_posicao_ver_y), NULL);
    
    
    //abrir grafico velocidade
    GtkWidget *abrir_grafico_velocidade;
    abrir_grafico_velocidade = gtk_menu_item_new_with_label("Abrir gráfico v(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_velocidade);
    g_signal_connect(G_OBJECT(abrir_grafico_velocidade), "activate", G_CALLBACK(fc_abrir_grafico_velocidade), window_grafico_velocidade);

    //hbox grafico velocidade
    GtkWidget *hbox_grafico_velocidade;
    hbox_grafico_velocidade = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window_grafico_velocidade), hbox_grafico_velocidade);

        //drawing area grafico velocidade
    darea_grafico_velocidade = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(hbox_grafico_velocidade), darea_grafico_velocidade, TRUE, TRUE, 0);

        //frame grafico velocidade opcoes
    GtkWidget *frame_grafico_velocidade_opcoes;
    frame_grafico_velocidade_opcoes = gtk_frame_new("Opcões");
    gtk_frame_set_label_align(GTK_FRAME(frame_grafico_velocidade_opcoes), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_grafico_velocidade_opcoes, 10);
    gtk_widget_set_margin_end(frame_grafico_velocidade_opcoes, 10);
    gtk_box_pack_start(GTK_BOX(hbox_grafico_velocidade), frame_grafico_velocidade_opcoes, FALSE, FALSE, 0);
 
        //box grafico velocidade opcoes
    GtkWidget *box_grafico_velocidade_opcoes;
    box_grafico_velocidade_opcoes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(box_grafico_velocidade_opcoes, 200, -1);
    gtk_container_add(GTK_CONTAINER(frame_grafico_velocidade_opcoes), box_grafico_velocidade_opcoes);

        //box grafico velocidade escala x
    GtkWidget *box_grafico_velocidade_escala_x;
    box_grafico_velocidade_escala_x = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_velocidade_opcoes), box_grafico_velocidade_escala_x, FALSE, FALSE, 5);

        //label grafico velocidade escala x
    GtkWidget *label_grafico_velocidade_escala_x;
    label_grafico_velocidade_escala_x = gtk_label_new("Escala x");
    gtk_widget_set_margin_start(label_grafico_velocidade_escala_x, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_velocidade_escala_x), label_grafico_velocidade_escala_x, FALSE, FALSE, 0);

        //scale grafico velocidade escala x
    GtkWidget  *scale_grafico_velocidade_escala_x;
    scale_grafico_velocidade_escala_x = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_velocidade_escala_x), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_velocidade_escala_x), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_velocidade_escala_x, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_velocidade_escala_x), scale_grafico_velocidade_escala_x, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_velocidade_escala_x), opcoes.grafico_velocidade_escala_x);
    g_signal_connect(G_OBJECT(scale_grafico_velocidade_escala_x), "value_changed", G_CALLBACK(fc_scale_grafico_velocidade_escala_x), NULL);

        //box grafico velocidade escala y
    GtkWidget *box_grafico_velocidade_escala_y;
    box_grafico_velocidade_escala_y = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_velocidade_opcoes), box_grafico_velocidade_escala_y, FALSE, FALSE, 5);

        //label grafico velocidade escala y
    GtkWidget *label_grafico_velocidade_escala_y;
    label_grafico_velocidade_escala_y = gtk_label_new("Escala y");
    gtk_widget_set_margin_start(label_grafico_velocidade_escala_y, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_velocidade_escala_y), label_grafico_velocidade_escala_y, FALSE, FALSE, 0);

        //scale grafico velocidade escala y
    GtkWidget  *scale_grafico_velocidade_escala_y;
    scale_grafico_velocidade_escala_y = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_velocidade_escala_y), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_velocidade_escala_y), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_velocidade_escala_y, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_velocidade_escala_y), scale_grafico_velocidade_escala_y, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_velocidade_escala_y), opcoes.grafico_velocidade_escala_y);
    g_signal_connect(G_OBJECT(scale_grafico_velocidade_escala_y), "value_changed", G_CALLBACK(fc_scale_grafico_velocidade_escala_y), NULL);

        //check button grafico velocidade ver x
    GtkWidget *check_button_grafico_velocidade_ver_x;
    check_button_grafico_velocidade_ver_x = gtk_check_button_new_with_label("Ver gráfico da componente de x");
    gtk_box_pack_start(GTK_BOX(box_grafico_velocidade_opcoes), check_button_grafico_velocidade_ver_x, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_velocidade_ver_x), opcoes.grafico_velocidade_ver_x);
    g_signal_connect(G_OBJECT(check_button_grafico_velocidade_ver_x), "toggled", G_CALLBACK(fc_check_button_grafico_velocidade_ver_x), NULL);

        //check button grafico velocidade ver y
    GtkWidget *check_button_grafico_velocidade_ver_y;
    check_button_grafico_velocidade_ver_y = gtk_check_button_new_with_label("Ver gráfico da componente de y");
    gtk_box_pack_start(GTK_BOX(box_grafico_velocidade_opcoes), check_button_grafico_velocidade_ver_y, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_velocidade_ver_y), opcoes.grafico_velocidade_ver_y);
    g_signal_connect(G_OBJECT(check_button_grafico_velocidade_ver_y), "toggled", G_CALLBACK(fc_check_button_grafico_velocidade_ver_y), NULL);

    //abrir grafico acelaracao
    GtkWidget *abrir_grafico_acelaracao;
    abrir_grafico_acelaracao = gtk_menu_item_new_with_label("Abrir gráfico a(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_acelaracao);
    g_signal_connect(G_OBJECT(abrir_grafico_acelaracao), "activate", G_CALLBACK(fc_abrir_grafico_acelaracao), window_grafico_acelaracao);

    //hbox grafico acelaracao
    GtkWidget *hbox_grafico_acelaracao;
    hbox_grafico_acelaracao = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window_grafico_acelaracao), hbox_grafico_acelaracao);

        //drawing area grafico acelaracao
    darea_grafico_acelaracao = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(hbox_grafico_acelaracao), darea_grafico_acelaracao, TRUE, TRUE, 0);

        //frame grafico acelaracao opcoes
    GtkWidget *frame_grafico_acelaracao_opcoes;
    frame_grafico_acelaracao_opcoes = gtk_frame_new("Opcões");
    gtk_frame_set_label_align(GTK_FRAME(frame_grafico_acelaracao_opcoes), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_grafico_acelaracao_opcoes, 10);
    gtk_widget_set_margin_end(frame_grafico_acelaracao_opcoes, 10);
    gtk_box_pack_start(GTK_BOX(hbox_grafico_acelaracao), frame_grafico_acelaracao_opcoes, FALSE, FALSE, 0);
 
        //box grafico acelaracao opcoes
    GtkWidget *box_grafico_acelaracao_opcoes;
    box_grafico_acelaracao_opcoes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(box_grafico_acelaracao_opcoes, 200, -1);
    gtk_container_add(GTK_CONTAINER(frame_grafico_acelaracao_opcoes), box_grafico_acelaracao_opcoes);

        //box grafico acelaracao escala x
    GtkWidget *box_grafico_acelaracao_escala_x;
    box_grafico_acelaracao_escala_x = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_acelaracao_opcoes), box_grafico_acelaracao_escala_x, FALSE, FALSE, 5);

        //label grafico acelaracao escala x
    GtkWidget *label_grafico_acelaracao_escala_x;
    label_grafico_acelaracao_escala_x = gtk_label_new("Escala x");
    gtk_widget_set_margin_start(label_grafico_acelaracao_escala_x, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_acelaracao_escala_x), label_grafico_acelaracao_escala_x, FALSE, FALSE, 0);

        //scale grafico acelaracao escala x
    GtkWidget  *scale_grafico_acelaracao_escala_x;
    scale_grafico_acelaracao_escala_x = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_acelaracao_escala_x), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_acelaracao_escala_x), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_acelaracao_escala_x, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_acelaracao_escala_x), scale_grafico_acelaracao_escala_x, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_acelaracao_escala_x), opcoes.grafico_acelaracao_escala_x);
    g_signal_connect(G_OBJECT(scale_grafico_acelaracao_escala_x), "value_changed", G_CALLBACK(fc_scale_grafico_acelaracao_escala_x), NULL);

        //box grafico acelaracao escala y
    GtkWidget *box_grafico_acelaracao_escala_y;
    box_grafico_acelaracao_escala_y = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_acelaracao_opcoes), box_grafico_acelaracao_escala_y, FALSE, FALSE, 5);

        //label grafico acelaracao escala y
    GtkWidget *label_grafico_acelaracao_escala_y;
    label_grafico_acelaracao_escala_y = gtk_label_new("Escala y");
    gtk_widget_set_margin_start(label_grafico_acelaracao_escala_y, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_acelaracao_escala_y), label_grafico_acelaracao_escala_y, FALSE, FALSE, 0);

        //scale grafico acelaracao escala y
    GtkWidget  *scale_grafico_acelaracao_escala_y;
    scale_grafico_acelaracao_escala_y = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_acelaracao_escala_y), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_acelaracao_escala_y), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_acelaracao_escala_y, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_acelaracao_escala_y), scale_grafico_acelaracao_escala_y, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_acelaracao_escala_y), opcoes.grafico_acelaracao_escala_y);
    g_signal_connect(G_OBJECT(scale_grafico_acelaracao_escala_y), "value_changed", G_CALLBACK(fc_scale_grafico_acelaracao_escala_y), NULL);

        //check button grafico acelaracao ver x
    GtkWidget *check_button_grafico_acelaracao_ver_x;
    check_button_grafico_acelaracao_ver_x = gtk_check_button_new_with_label("Ver gráfico da componente de x");
    gtk_box_pack_start(GTK_BOX(box_grafico_acelaracao_opcoes), check_button_grafico_acelaracao_ver_x, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_acelaracao_ver_x), opcoes.grafico_acelaracao_ver_x);
    g_signal_connect(G_OBJECT(check_button_grafico_acelaracao_ver_x), "toggled", G_CALLBACK(fc_check_button_grafico_acelaracao_ver_x), NULL);

        //check button grafico acelaracao ver y
    GtkWidget *check_button_grafico_acelaracao_ver_y;
    check_button_grafico_acelaracao_ver_y = gtk_check_button_new_with_label("Ver gráfico da componente de y");
    gtk_box_pack_start(GTK_BOX(box_grafico_acelaracao_opcoes), check_button_grafico_acelaracao_ver_y, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_acelaracao_ver_y), opcoes.grafico_acelaracao_ver_y);
    g_signal_connect(G_OBJECT(check_button_grafico_acelaracao_ver_y), "toggled", G_CALLBACK(fc_check_button_grafico_acelaracao_ver_y), NULL);

    //abrir grafico energia
    GtkWidget *abrir_grafico_energia;
    abrir_grafico_energia = gtk_menu_item_new_with_label("Abrir gráfico E(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_energia);
    g_signal_connect(G_OBJECT(abrir_grafico_energia), "activate", G_CALLBACK(fc_abrir_grafico_energia), window_grafico_energia);

        //hbox grafico energia
    GtkWidget *hbox_grafico_energia;
    hbox_grafico_energia = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window_grafico_energia), hbox_grafico_energia);

        //drawing area grafico energia
    darea_grafico_energia = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(hbox_grafico_energia), darea_grafico_energia, TRUE, TRUE, 0);

        //frame grafico energia opcoes
    GtkWidget *frame_grafico_energia_opcoes;
    frame_grafico_energia_opcoes = gtk_frame_new("Opcões");
    gtk_frame_set_label_align(GTK_FRAME(frame_grafico_energia_opcoes), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_grafico_energia_opcoes, 10);
    gtk_widget_set_margin_end(frame_grafico_energia_opcoes, 10);
    gtk_box_pack_start(GTK_BOX(hbox_grafico_energia), frame_grafico_energia_opcoes, FALSE, FALSE, 0);
 
        //box grafico energia opcoes
    GtkWidget *box_grafico_energia_opcoes;
    box_grafico_energia_opcoes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(box_grafico_energia_opcoes, 200, -1);
    gtk_container_add(GTK_CONTAINER(frame_grafico_energia_opcoes), box_grafico_energia_opcoes);

        //box grafico energia escala x
    GtkWidget *box_grafico_energia_escala_x;
    box_grafico_energia_escala_x = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_opcoes), box_grafico_energia_escala_x, FALSE, FALSE, 5);

        //label grafico energia escala x
    GtkWidget *label_grafico_energia_escala_x;
    label_grafico_energia_escala_x = gtk_label_new("Escala x");
    gtk_widget_set_margin_start(label_grafico_energia_escala_x, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_escala_x), label_grafico_energia_escala_x, FALSE, FALSE, 0);

        //scale grafico energia escala x
    GtkWidget  *scale_grafico_energia_escala_x;
    scale_grafico_energia_escala_x = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_energia_escala_x), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_energia_escala_x), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_energia_escala_x, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_energia_escala_x), scale_grafico_energia_escala_x, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_energia_escala_x), opcoes.grafico_energia_escala_x);
    g_signal_connect(G_OBJECT(scale_grafico_energia_escala_x), "value_changed", G_CALLBACK(fc_scale_grafico_energia_escala_x), NULL);

        //box grafico energia escala y
    GtkWidget *box_grafico_energia_escala_y;
    box_grafico_energia_escala_y = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_opcoes), box_grafico_energia_escala_y, FALSE, FALSE, 5);

        //label grafico energia escala y
    GtkWidget *label_grafico_energia_escala_y;
    label_grafico_energia_escala_y = gtk_label_new("Escala y");
    gtk_widget_set_margin_start(label_grafico_energia_escala_y, 10);
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_escala_y), label_grafico_energia_escala_y, FALSE, FALSE, 0);

        //scale grafico energia escala y
    GtkWidget  *scale_grafico_energia_escala_y;
    scale_grafico_energia_escala_y = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GRAFICO_ESCALA_MIN, GRAFICO_ESCALA_MAX, 0.01);
    gtk_scale_set_digits(GTK_SCALE(scale_grafico_energia_escala_y), 2);
    gtk_scale_set_value_pos(GTK_SCALE(scale_grafico_energia_escala_y), GTK_POS_LEFT);
    gtk_widget_set_margin_start(scale_grafico_energia_escala_y, 20);
    gtk_box_pack_end(GTK_BOX(box_grafico_energia_escala_y), scale_grafico_energia_escala_y, TRUE, TRUE, 0);
    gtk_range_set_value(GTK_RANGE(scale_grafico_energia_escala_y), opcoes.grafico_energia_escala_y);
    g_signal_connect(G_OBJECT(scale_grafico_energia_escala_y), "value_changed", G_CALLBACK(fc_scale_grafico_energia_escala_y), NULL);

        //check button grafico eneriga ver potencial eletrico
    GtkWidget *check_button_grafico_energia_ver_potencial_eletrico;
    check_button_grafico_energia_ver_potencial_eletrico = gtk_check_button_new_with_label("Ver gráfico da energia potencial eletrica");
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_opcoes), check_button_grafico_energia_ver_potencial_eletrico, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_energia_ver_potencial_eletrico), opcoes.grafico_energia_ver_potencial_eletrico);
    g_signal_connect(G_OBJECT(check_button_grafico_energia_ver_potencial_eletrico), "toggled", G_CALLBACK(fc_check_button_grafico_energia_ver_potencial_eletrico), NULL);

        //check button grafico eneriga ver potencial magnetico
    GtkWidget *check_button_grafico_energia_ver_potencial_magnetico;
    check_button_grafico_energia_ver_potencial_magnetico = gtk_check_button_new_with_label("Ver gráfico da energia potencial magnetica");
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_opcoes), check_button_grafico_energia_ver_potencial_magnetico, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_energia_ver_potencial_magnetico), opcoes.grafico_energia_ver_potencial_magnetico);
    g_signal_connect(G_OBJECT(check_button_grafico_energia_ver_potencial_magnetico), "toggled", G_CALLBACK(fc_check_button_grafico_energia_ver_potencial_magnetico), NULL);

        //check button grafico eneriga ver cinetica
    GtkWidget *check_button_grafico_energia_ver_cinetica;
    check_button_grafico_energia_ver_cinetica = gtk_check_button_new_with_label("Ver gráfico da energia cinetica");
    gtk_box_pack_start(GTK_BOX(box_grafico_energia_opcoes), check_button_grafico_energia_ver_cinetica, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_grafico_energia_ver_cinetica), opcoes.grafico_energia_ver_cinetica);
    g_signal_connect(G_OBJECT(check_button_grafico_energia_ver_cinetica), "toggled", G_CALLBACK(fc_check_button_grafico_energia_ver_cinetica), NULL);

//////////////////////// TEMA ////////////////////

    GtkWidget *item_tema;
    item_tema = gtk_menu_item_new_with_label("Tema");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_tema);

    GtkWidget *menu_tema;
    menu_tema = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_tema), menu_tema);
    
    GSList *tema = NULL;

    //tema claro
    GtkWidget *tema_claro;
    tema_claro = gtk_radio_menu_item_new_with_label(tema, "Claro");
    tema = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(tema_claro));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_tema), tema_claro);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(tema_claro), TRUE);

    //tema escuro
    GtkWidget *tema_escuro;
    tema_escuro = gtk_radio_menu_item_new_with_label(tema, "Escuro");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_tema), tema_escuro);

//////////////////////// SOBRE ////////////////////

    GtkWidget *item_sobre;
    item_sobre = gtk_menu_item_new_with_label("Sobre");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_sobre);

//////////////////////// FECHAR ////////////////////

    GtkWidget *item_fechar;
    item_fechar = gtk_menu_item_new_with_label("Fechar");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_fechar);
    g_signal_connect(G_OBJECT(item_fechar), "activate", G_CALLBACK(fechar_programa), NULL);

//////////////////////// FIM ////////////////////

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), darea);
    g_signal_connect(G_OBJECT(darea_grafico_posicao), "draw", G_CALLBACK(on_draw_event_grafico_posicao), darea);
    g_signal_connect(G_OBJECT(darea_grafico_velocidade), "draw", G_CALLBACK(on_draw_event_grafico_velocidade), darea);
    g_signal_connect(G_OBJECT(darea_grafico_acelaracao), "draw", G_CALLBACK(on_draw_event_grafico_acelaracao), darea);
    g_signal_connect(G_OBJECT(darea_grafico_energia), "draw", G_CALLBACK(on_draw_event_grafico_energia), darea);
    
    //ciclo timeout
    g_timeout_add (30, (GSourceFunc) time_handler, darea);
    g_timeout_add (30, (GSourceFunc) time_handler, darea_grafico_posicao);
    g_timeout_add (30, (GSourceFunc) time_handler, darea_grafico_velocidade);
    g_timeout_add (30, (GSourceFunc) time_handler, darea_grafico_acelaracao);
    g_timeout_add (30, (GSourceFunc) time_handler, darea_grafico_energia);
    
    //gtk main
    gtk_widget_show_all(window);
    gtk_widget_show_all(window_opcoes);
    gtk_main();
    return 0;
}
