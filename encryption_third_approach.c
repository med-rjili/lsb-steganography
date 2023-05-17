#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

typedef unsigned char byte;

// Fonction pour cacher un texte dans une image
void cacher_texte(byte *pixels, int taille_pixels, char *texte) {
    int i, j, k;
    int taille_texte = strlen(texte);
    for (int i = 0; i < 3; i++) {
        pixels[i] = (taille_texte >> (5 * (2 - i)) & 31);
    }
    byte bit;
    for (i = 0; i < taille_texte; i++) {
        pixels[2*i+3] = pixels[2*i+3] | ((texte[i] >> 4 )& 15);
        pixels[2*i+4] = pixels[2*i+4] | (texte[i] & 15);
    }
}

// Fonction pour extraire un texte d'une image
char *extraire_texte(byte *pixels, int taille_pixels) {
    int i, j, k;
    int taille_texte = 0;
    for (int i = 0; i < 3; i++) {
        taille_texte += (pixels[i] << (5 * (2 - i)));
    }
    char *texte = malloc(taille_texte + 1);
    byte bit;
    for (i = 0; i < taille_texte; i++) {
        // Initialiser le caractère à zéro
        texte[i] = 0;
        texte[i]+= (pixels[2*i+3] & 15)<<4;
        texte[i]+= (pixels[2*i+4] & 15);
    }
    // Terminer la chaîne de caractères
    texte[taille_texte] = '\0';
    return texte;
}

// Fonction principale
int main(int argc, char *argv[]) {
    if (argc>4 || argc <3) {
        printf("Usage: %s <image> <texte> <image-sortie> if u want to encrypt\n ", argv[0]);
        printf("%s <image> <texte> if u want to decrypt\n ", argv[0]);
        return 1;
    }
    // Ouvrir l'image d'entrée
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Erreur: impossible d'ouvrir l'image %s\n", argv[1]);
        return 1;
    }
    // Lire les pixels de l'image PNG
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        printf("Erreur: impossible de créer la structure de lecture PNG\n");
        fclose(f);
        return 1;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("Erreur: impossible de créer la structure d'information PNG\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return 1;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
    printf("Erreur: impossible de lire l'image PNG %s\n", argv[1]);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);
    return 1;
}
png_init_io(png_ptr, f);
png_read_info(png_ptr, info_ptr);

// Récupérer les informations de l'image PNG
int width = png_get_image_width(png_ptr, info_ptr);
int height = png_get_image_height(png_ptr, info_ptr);
png_byte color_type = png_get_color_type(png_ptr, info_ptr);
png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

// Convertir l'image PNG en tableau de pixels RGBA
if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr);
if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);
if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);
if (bit_depth == 16)
    png_set_scale_16(png_ptr);
if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);
png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
png_read_update_info(png_ptr, info_ptr);

int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
for (int y = 0; y < height; y++)
    row_pointers[y] = (png_byte *)malloc(rowbytes);
png_read_image(png_ptr, row_pointers);



if(argc==4){
// Lire le texte à cacher dans l'image
FILE *text_file = fopen(argv[2], "r");
if (!text_file) {
    printf("Erreur: impossible d'ouvrir le fichier de texte %s\n", argv[2]);
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);
    return 1;
}
fseek(text_file, 0, SEEK_END);
long text_size = ftell(text_file);
rewind(text_file);
char *text = malloc(text_size + 1);
fread(text,text_size,1, text_file);
fclose(text_file);
text[text_size] = '\0';
// Cacher le texte dans l'image
cacher_texte(row_pointers[0], width * height * 4, text);

// Sauvegarder l'image modifiée
FILE *out_file = fopen(argv[3], "wb");
if (!out_file) {
    printf("Erreur: impossible de créer le fichier de sortie %s\n", argv[3]);
    free(text);
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);
    return 1;
}
png_structp png_out_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
if (!png_out_ptr) {
    printf("Erreur: impossible de créer la structure d'écriture PNG\n");
    free(text);
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);
    fclose(out_file);
    return 1;
}
png_infop out_info_ptr = png_create_info_struct(png_out_ptr);
if (!out_info_ptr) {
    printf("Erreur: impossible de créer la structure d'information PNG\n");
    free(text);
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    png_destroy_write_struct(&png_out_ptr, NULL);
    fclose(f);
    fclose(out_file);
    return 1;
}
if (setjmp(png_jmpbuf(png_out_ptr))) {
    printf("Erreur: impossible d'écrire l'image PNG %s\n", argv[3]);
    free(text);
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    png_destroy_write_struct(&png_out_ptr, &out_info_ptr);
    fclose(f);
    fclose(out_file);
    return 1;
}
png_init_io(png_out_ptr, out_file);
png_set_IHDR(png_out_ptr, out_info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
png_write_info(png_out_ptr, out_info_ptr);
png_write_image(png_out_ptr, row_pointers);
png_write_end(png_out_ptr, NULL);
for (int y = 0; y < height; y++)
    free(row_pointers[y]);
free(row_pointers);
png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
png_destroy_write_struct(&png_out_ptr, &out_info_ptr);
fclose(f);
fclose(out_file);
free(text);
}
else
{
char* text_rec="";
text_rec=extraire_texte(row_pointers[0], width * height * 4);
char *filename = argv[2];

    // Ouvre le fichier en mode écriture
    FILE *fp = fopen(filename, "w");

    // Vérifie si le fichier a bien été ouvert
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    // Écrit le texte dans le fichier
    fprintf(fp, "%s", text_rec);

    // Ferme le fichier
    fclose(fp);

return 0;

}
}


