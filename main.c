#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <math.h>

#include <gtk/gtk.h>
#include <cairo.h>

#define MAX_X 10
#define MAX_Y 10
#define MAX_VEL 10
#define MAX_CARGA 10
#define MAX_MASSA 10
#define MAX_INTENSIDADE_CAMPO_MAGNETICO 10
#define MAX_INTENSIDADE_CAMPO_ELETRICO 10

gdouble dt = 0, dt_temp = 1.5;
gint width = 1080, height = 850;

GtkWidget *window, *darea;

//////////////////////////////////////////// PERSONAL_MATH ////////////////////////////////////////////

//////////////////////// personal_math.h ////////////////////
#define quadrado(x) ((x)*(x))

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

vetor vetor_somar(vetor *v1, vetor *v2)
{
    vetor v3 = {v1->x + v2->x,
                v1->y + v2->y,
                v1->z + v2->z};
    return v3;
}

vetor vetor_escalar(double escalar, vetor *v1)
{
    vetor v3 = {escalar * v1->x,
                escalar * v1->y,
                escalar * v1->z};
    return v3;
}

vetor vetor_externo(vetor *v1, vetor *v2)
{
    vetor v3 = {v1->y * v2->z - v1->z * v2->y,
                v1->z * v2->x - v1->x * v2->z,
                v1->x * v2->y - v1->y * v2->x};
    return v3;
}

double vetor_interno(vetor *v1, vetor *v2)
{
    return sqrt( quadrado(v1->x - v2->x) + quadrado(v1->y - v2->y) + quadrado(v1->z - v2->z) );
}


//////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////

//////////////////////// functions.h ////////////////////

typedef struct estrutura_particula
{
    vetor r, r0, v, a, F;
    double carga, massa;
    double angulo_velocidade_inicial, intensidade_velocidade_inicial;
}estrutura_particula;

typedef struct estrutura_campo_magnetico
{
    vetor B;
    int sentido; //sinal componete no eixo z
    double intensidade; //componente no eixo z
}estrutura_campo_magnetico;

typedef struct estrutura_campo_eletrico //se for uniforme nao existe origem; se nao for nao existe angulo
{
    gboolean e_uniforme;
    vetor origem;
    double angulo, intensidade; //intensidade "e a carga da particula geradora"    
}estrutura_campo_eletrico;

typedef enum tenum_temaema{claro, escuro} enum_tema;
typedef enum enum_tipo_colisao{elastica, pausar} enum_tipo_colisao;

typedef struct estrutura_opcoes
{
    gboolean ver_referencial, 
            ver_forca,
            ver_acelaracao,
            ver_velocidade,
            ver_posicao,
            ver_posicao_inicial,
            ver_campo_eletrico,
            ver_campo_magnetico,
            ver_tragetoria,
            campo_eletrico_unicorme,

            abrir_grafico_posicao,
            abrir_grafico_velocidade,
            abrir_grafico_acelaracao,
            abrir_grafico_energia;
    enum_tipo_colisao tipo_colisao;
    enum_tema tema;
}estrutura_opcoes;

//////////////////////// functions.c ////////////////////

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
    particula.v = vetor_escalar(intensidade_velocidade_inicial, &particula.v);
    particula.a = vetor_criar(0, 0, 0);
    particula.F = vetor_criar(0, 0, 0);
    
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

estrutura_opcoes criar_opcoes(gboolean ver_referencial, 
                            gboolean ver_forca,
                            gboolean ver_acelaracao,
                            gboolean ver_velocidade,
                            gboolean ver_posicao,
                            gboolean ver_posicao_inicial,
                            gboolean ver_campo_eletrico,
                            gboolean ver_campo_magnetico,
                            gboolean ver_tragetoria,
                            gboolean campo_eletrico_unicorme,
                            gboolean abrir_grafico_posicao,
                            gboolean abrir_grafico_velocidade,
                            gboolean abrir_grafico_acelaracao,
                            gboolean abrir_grafico_energia,
                            enum_tipo_colisao tipo_colisao,
                            enum_tema tema)
{
    estrutura_opcoes opcoes;

    opcoes.ver_referencial = ver_referencial;
    opcoes.ver_forca = ver_forca;
    opcoes.ver_acelaracao = ver_acelaracao;
    opcoes.ver_velocidade = ver_velocidade;
    opcoes.ver_posicao = ver_posicao;
    opcoes.ver_posicao_inicial = ver_posicao_inicial;
    opcoes.ver_campo_eletrico = ver_campo_eletrico;
    opcoes.ver_campo_magnetico = ver_campo_magnetico;
    opcoes.ver_tragetoria = ver_tragetoria;
    opcoes.campo_eletrico_unicorme = campo_eletrico_unicorme;
    opcoes.abrir_grafico_posicao = abrir_grafico_posicao;
    opcoes.abrir_grafico_velocidade = abrir_grafico_velocidade;
    opcoes.abrir_grafico_acelaracao = abrir_grafico_acelaracao;
    opcoes.abrir_grafico_energia = abrir_grafico_energia;
    opcoes.tipo_colisao = tipo_colisao;
    opcoes.tema = tema;

    return opcoes;
}

estrutura_particula particula;
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
    campo_magnetico.sentido *= -1;
    return FALSE;
}

gboolean fc_spin_button_intensidade_campo_magnetico(GtkWidget *w)
{
    campo_magnetico.intensidade =  gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
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
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_widget_set_sensitive(button_parar_tempo, FALSE);
    gtk_widget_set_sensitive(button_continuar_tempo, TRUE);
    return FALSE;
}

gboolean fc_button_continuar_tempo(GtkWidget *button_continuar_tempo, GtkWidget *button_parar_tempo)
{
    dt = dt_temp;
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_widget_set_sensitive(button_parar_tempo, TRUE);
    gtk_widget_set_sensitive(button_continuar_tempo, FALSE);
    return FALSE;
}

//////////////////////////////////////////// DRAWING ////////////////////////////////////////////

//TA PESSIMO E SO PARA TESTAAR SE DAVA AO MENOS
gboolean on_draw_event(GtkWidget *widget, cairo_t *cr)
{
    static gint darea_width = 0, darea_height = 0;
    darea_height = gtk_widget_get_allocated_height(darea);
    darea_width = gtk_widget_get_allocated_width(darea);


    static gdouble posx = 800/2;
    static gdouble posy = 800/2;

    cairo_set_source_rgb(cr, 0., 0.5, 0.5);
    cairo_set_line_width (cr, 4.0);
    cairo_arc (cr, posx, posy, 5, 0., 2 * M_PI);
    cairo_fill (cr);

    posx += dt * campo_magnetico.sentido * campo_magnetico.intensidade;
    posy += dt * campo_magnetico.sentido * campo_magnetico.intensidade;

    return FALSE;
}

gboolean time_handler (GtkWidget *widget)
{
    if ((!GTK_IS_WIDGET(widget)) || (!gtk_widget_get_window (widget)))
        return FALSE;

    gtk_widget_queue_draw(widget);

    return TRUE;
}

//////////////////////////////////////////// MAIN ////////////////////////////////////////////

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    gtk_init(&argc, &argv);

    //iniciar
    particula = criar_particula(vetor_criar(1,2,0), M_PI, 2, 1, 1);
    campo_magnetico = criar_campo_magnetico(1, 0);
    campo_eletrico = criar_campo_eletrico(TRUE, vetor_criar(-1, -2, 0), 0, -10);
    opcoes = criar_opcoes(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, elastica, claro);

    //criar janela
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Força de Lorentz");
    gtk_window_set_default_size(GTK_WINDOW(window), 1080,  800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //conectar botao de fecho
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

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
    gtk_box_pack_start(GTK_BOX(hbox1), frame_drawing_area, TRUE, TRUE, 0);

    //drawing area 
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(frame_drawing_area), darea);

//////////////////////// OPCOES ////////////////////

    //frame principal
    GtkWidget *frame_opcoes;
    frame_opcoes = gtk_frame_new("Opções");
    gtk_frame_set_label_align(GTK_FRAME(frame_opcoes), 0, 0.5);
    gtk_box_pack_end(GTK_BOX(hbox1), frame_opcoes, FALSE, FALSE, 0);

    //box opcoes
    GtkWidget *box_opcoes;
    box_opcoes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_opcoes), box_opcoes);

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
    box_variaveis = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(frame_variaveis), box_variaveis);

//////////////////////// PARTICULA ////////////////////

    //frame particula
    GtkWidget *frame_particula;
    frame_particula = gtk_frame_new("Partícula");
    gtk_frame_set_label_align(GTK_FRAME(frame_particula), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_particula, 10);
    gtk_widget_set_margin_end(frame_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_variaveis), frame_particula, FALSE, FALSE, 0);

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
    gtk_box_pack_start(GTK_BOX(box_particula), frame_posicao_inicial_particula, FALSE, FALSE, 0);

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
    g_signal_connect(G_OBJECT(spin_button_y_posicao_inicial_particula), "changed", G_CALLBACK(fc_spin_button_y_posicao_inicial_particula), NULL);

//////////////////////// VELOCIDADE INICIAL PARTICULA ////////////////////

    //frame velocidade inicial particula
    GtkWidget *frame_velocidade_inicial_particula;
    frame_velocidade_inicial_particula = gtk_frame_new("Velocidade inicial");
    gtk_frame_set_label_align(GTK_FRAME(frame_velocidade_inicial_particula), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_velocidade_inicial_particula, 10);
    gtk_widget_set_margin_end(frame_velocidade_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_particula), frame_velocidade_inicial_particula, FALSE, FALSE, 0);

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
    gtk_range_set_value(GTK_RANGE(scale_angulo_velocidade_inicial_particula), 0);
    gtk_widget_set_margin_start(scale_angulo_velocidade_inicial_particula, 20);
    gtk_box_pack_end(GTK_BOX(box_angulo_velocidade_inicial_particula), scale_angulo_velocidade_inicial_particula, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(scale_angulo_velocidade_inicial_particula), "value_changed", G_CALLBACK(fc_scale_angulo_velocidade_inicial_particula), NULL);


    //box intensidade velocidade inicial particula
    GtkWidget *box_intensidade_velocidade_inicial_particula;
    box_intensidade_velocidade_inicial_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_velocidade_inicial_particula), box_intensidade_velocidade_inicial_particula, FALSE, FALSE, 5);

    //label intensidade velocidade inicial particula
    GtkWidget *label_intensidade_velocidade_inicial_particula;
    label_intensidade_velocidade_inicial_particula = gtk_label_new("Intensidade");
    gtk_widget_set_margin_start(label_intensidade_velocidade_inicial_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_intensidade_velocidade_inicial_particula), label_intensidade_velocidade_inicial_particula, FALSE, FALSE, 0);

    //spin_button intensidade velocidade inicial particula
    GtkWidget  *spin_button_intensidade_velocidade_inicial_particula;
    spin_button_intensidade_velocidade_inicial_particula = gtk_spin_button_new_with_range(0, MAX_VEL, 0.01);
    gtk_box_pack_end(GTK_BOX(box_intensidade_velocidade_inicial_particula), spin_button_intensidade_velocidade_inicial_particula, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(spin_button_intensidade_velocidade_inicial_particula), "changed", G_CALLBACK(fc_spin_button_intensidade_velocidade_inicial_particula), NULL);

    //box carga particula
    GtkWidget *box_carga_particula;
    box_carga_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_particula), box_carga_particula, FALSE, FALSE, 5);

    //label carga particula
    GtkWidget *label_carga_particula;
    label_carga_particula = gtk_label_new("Carga");
    gtk_widget_set_margin_start(label_carga_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_carga_particula), label_carga_particula, FALSE, FALSE, 0);

    //spin_button carga particula
    GtkWidget  *spin_button_carga_particula;
    spin_button_carga_particula = gtk_spin_button_new_with_range(-MAX_CARGA, MAX_CARGA, 0.01);
    gtk_box_pack_end(GTK_BOX(box_carga_particula), spin_button_carga_particula, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(spin_button_carga_particula), "changed", G_CALLBACK(fc_spin_button_carga_particula), NULL);

    //box massa particula
    GtkWidget *box_massa_particula;
    box_massa_particula = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box_particula), box_massa_particula, FALSE, FALSE, 5);

    //label x massa particula
    GtkWidget *label_massa_particula;
    label_massa_particula = gtk_label_new("Massa");
    gtk_widget_set_margin_start(label_massa_particula, 10);
    gtk_box_pack_start(GTK_BOX(box_massa_particula), label_massa_particula, FALSE, FALSE, 0);

    //spin_button x massa particula
    GtkWidget  *spin_button_massa_particula;
    spin_button_massa_particula = gtk_spin_button_new_with_range(0, MAX_MASSA, 0.01);
    gtk_box_pack_end(GTK_BOX(box_massa_particula), spin_button_massa_particula, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(spin_button_massa_particula), "changed", G_CALLBACK(fc_spin_button_massa_particula), NULL);

//////////////////////// CAMPO MAGNETICO ////////////////////

    //frame campo magnetico
    GtkWidget *frame_campo_magnetico;
    frame_campo_magnetico = gtk_frame_new("Campo magnético");
    gtk_frame_set_label_align(GTK_FRAME(frame_campo_magnetico), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_campo_magnetico, 10);
    gtk_widget_set_margin_end(frame_campo_magnetico, 10);
    gtk_box_pack_start(GTK_BOX(box_variaveis), frame_campo_magnetico, FALSE, FALSE, 0);

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
    g_signal_connect(G_OBJECT(spin_button_intensidade_campo_magnetico), "changed", G_CALLBACK(fc_spin_button_intensidade_campo_magnetico), NULL);

//////////////////////// CAMPO ELETRICO ////////////////////

    //frame campo eletrico
    GtkWidget *frame_campo_eletrico;
    frame_campo_eletrico = gtk_frame_new("Campo elétrico");
    gtk_frame_set_label_align(GTK_FRAME(frame_campo_eletrico), 0.1, 0.5);
    gtk_widget_set_margin_start(frame_campo_eletrico, 10);
    gtk_widget_set_margin_end(frame_campo_eletrico, 10);
    gtk_box_pack_start(GTK_BOX(box_variaveis), frame_campo_eletrico, FALSE, FALSE, 0);

    //box campo magnetico
    GtkWidget *box_campo_eletrico;
    box_campo_eletrico = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_campo_eletrico), box_campo_eletrico);

//////////////////////// ORIGEM CAMPO ELETRICO ////////////////////

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
    gtk_range_set_value(GTK_RANGE(scale_angulo_campo_eletrico), 0);
    gtk_widget_set_margin_start(scale_angulo_campo_eletrico, 20);
    gtk_box_pack_end(GTK_BOX(box_angulo_campo_eletrico), scale_angulo_campo_eletrico, TRUE, TRUE, 0);
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
    g_signal_connect(G_OBJECT(spin_button_intensidade_campo_eletrico), "changed", G_CALLBACK(fc_spin_button_intensidade_campo_eletrico), NULL);
    
    //frame controlo tempo
    GtkWidget *frame_controlo_tempo;
    frame_controlo_tempo = gtk_frame_new("Controlo do tempo");
    gtk_frame_set_label_align(GTK_FRAME(frame_controlo_tempo), 0.05, 0.5);
    gtk_widget_set_margin_start(frame_controlo_tempo, 10);
    gtk_widget_set_margin_end(frame_controlo_tempo, 10);
    gtk_box_pack_start(GTK_BOX(box_opcoes), frame_controlo_tempo, FALSE, FALSE, 0);

    //box controlo tempo
    GtkWidget *box_controlo_tempo;
    box_controlo_tempo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(frame_controlo_tempo), box_controlo_tempo);

    //button parar tempo
    GtkWidget *button_parar_tempo, *button_continuar_tempo;
    button_parar_tempo = gtk_button_new_with_label("Parar");
    gtk_box_pack_start(GTK_BOX(box_controlo_tempo), button_parar_tempo, TRUE, TRUE, 0);
    gtk_widget_set_margin_top(button_parar_tempo, 5);
    gtk_widget_set_margin_bottom(button_parar_tempo, 5);
    
    //button continuar tempo
    button_continuar_tempo = gtk_button_new_with_label("Continuar");
    gtk_box_pack_start(GTK_BOX(box_controlo_tempo), button_continuar_tempo, TRUE, TRUE, 0);
    gtk_widget_set_margin_top(button_continuar_tempo, 5);
    gtk_widget_set_margin_bottom(button_continuar_tempo, 5);

    g_signal_connect(G_OBJECT(button_parar_tempo), "pressed", G_CALLBACK(fc_button_parar_tempo), button_continuar_tempo);
    g_signal_connect(G_OBJECT(button_continuar_tempo), "pressed", G_CALLBACK(fc_button_continuar_tempo), button_parar_tempo);

    fc_button_continuar_tempo(button_continuar_tempo, button_parar_tempo);

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

    //ver referencial
    GtkWidget *ver_referencial;
    ver_referencial = gtk_check_menu_item_new_with_label("Ver referencial");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_referencial);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ver_referencial), TRUE);

    //ver F
    GtkWidget *ver_forca;
    ver_forca = gtk_check_menu_item_new_with_label("Ver vetor força");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_forca);

    //ver a
    GtkWidget *ver_acelaracao;
    ver_acelaracao = gtk_check_menu_item_new_with_label("Ver vetor acelaração");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_acelaracao);

    //ver v
    GtkWidget *ver_velocidade;
    ver_velocidade = gtk_check_menu_item_new_with_label("Ver vetor velocidade");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_velocidade);

    //ver r
    GtkWidget *ver_posicao;
    ver_posicao = gtk_check_menu_item_new_with_label("Ver vetor posição");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_posicao);

    //ver r0
    GtkWidget *ver_posicao_inicial;
    ver_posicao_inicial = gtk_check_menu_item_new_with_label("Ver vetor posição inicial");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_posicao_inicial);

    //ver B
    GtkWidget *ver_campo_magnetico;
    ver_campo_magnetico = gtk_check_menu_item_new_with_label("Ver vetor campo magnético");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_campo_magnetico);

    //ver E
    GtkWidget *ver_campo_eletrico;
    ver_campo_eletrico = gtk_check_menu_item_new_with_label("Ver vetor campo elétrico");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_campo_eletrico);

    //separador
    GtkWidget *separador1;
    separador1 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), separador1);

    //ver tragetoria
    GtkWidget *ver_tragetoria;
    ver_tragetoria = gtk_check_menu_item_new_with_label("Ver tragetória");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), ver_tragetoria);

    //campo eletrico uniforme
    GtkWidget *campo_eletrico_uniforme;
    campo_eletrico_uniforme = gtk_check_menu_item_new_with_label("Campo elétrico uniforme");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_visualizacao), campo_eletrico_uniforme);

//////////////////////// VISUALIZACAO ////////////////////

    GtkWidget *item_abrir_graficos;
    item_abrir_graficos = gtk_menu_item_new_with_label("Abrir gráficos");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_abrir_graficos);

    GtkWidget *menu_abrir_graficos;
    menu_abrir_graficos = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_abrir_graficos), menu_abrir_graficos);

    //abrir grafico posicao
    GtkWidget *abrir_grafico_posicao;
    abrir_grafico_posicao = gtk_check_menu_item_new_with_label("Abrir gráfico r(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_posicao);

    //abrir grafico velocidade
    GtkWidget *abrir_grafico_velocidade;
    abrir_grafico_velocidade = gtk_check_menu_item_new_with_label("Abrir gráfico v(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_velocidade);

    //abrir grafico acelaracao
    GtkWidget *abrir_grafico_acelaracao;
    abrir_grafico_acelaracao = gtk_check_menu_item_new_with_label("Abrir gráfico a(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_acelaracao);

    //abrir grafico energia
    GtkWidget *abrir_grafico_energia;
    abrir_grafico_energia = gtk_check_menu_item_new_with_label("Abrir gráfico E(t)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_abrir_graficos), abrir_grafico_energia);

//////////////////////// borda ////////////////////

    GtkWidget *item_borda;
    item_borda = gtk_menu_item_new_with_label("Interação com a borda");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item_borda);

    GtkWidget *menu_borda;
    menu_borda = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_borda), menu_borda);
    
    GSList *opcoes_borda = NULL;

    //colicao elastica
    GtkWidget *borda_colicao_elastica;
    borda_colicao_elastica = gtk_radio_menu_item_new_with_label(opcoes_borda, "Colição elástica");
    opcoes_borda = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(borda_colicao_elastica));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_borda), borda_colicao_elastica);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(borda_colicao_elastica), TRUE);

    //para simulacao
    GtkWidget *borda_parar_simulacao;
    borda_parar_simulacao = gtk_radio_menu_item_new_with_label(opcoes_borda, "Parar simulação");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_borda), borda_parar_simulacao);

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

//////////////////////// FIM ////////////////////

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
    
    //ciclo timeout
    g_timeout_add (10, (GSourceFunc) time_handler, darea);
    
    //gtk main
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}