typedef struct 
{
    float x1;
    float x2;
    float y1;
    float y2;
} line;

line unscaled_win_text[] = {{
                                .x1 = 0.1,
                                .y1 = 0.9,
                                .x2 = 0.2,
                                .y2 = 0.7
                            },
                            {
                                .x1 = 0.2,
                                .y1 = 0.7,
                                .x2 = 0.3,
                                .y2 = 0.9
                            },
                            {
                                .x1 = 0.2,
                                .y1 = 0.7,
                                .x2 = 0.2,
                                .y2 = 0.5
                            },              // y
                            {
                                .x1 = 0.4,
                                .y1 = 0.9,
                                .x2 = 0.6,
                                .y2 = 0.9
                            },
                            {
                                .x1 = 0.4,
                                .y1 = 0.5,
                                .x2 = 0.6,
                                .y2 = 0.5
                            },
                            {
                                .x1 = 0.4,
                                .y1 = 0.9,
                                .x2 = 0.4,
                                .y2 = 0.5
                            },
                            {
                                .x1 = 0.6,
                                .y1 = 0.9,
                                .x2 = 0.6,
                                .y2 = 0.5
                            },            // o
                            {
                                .x1 = 0.7,
                                .y1 = 0.5,
                                .x2 = 0.9,
                                .y2 = 0.5
                            },
                            {
                                .x1 = 0.7,
                                .y1 = 0.9,
                                .x2 = 0.7,
                                .y2 = 0.5
                            },
                            {
                                .x1 = 0.9,
                                .y1 = 0.9,
                                .x2 = 0.9,
                                .y2 = 0.5
                            }           // u
                            }; 

line * to_screen_coords(line unscaled[], unsigned short int arr_size, float width, float height){
    line (*p_scaled)[] = malloc(arr_size);
    for (unsigned short i = 0; i < arr_size/sizeof(line); i++){
        (*p_scaled)[i] = unscaled[i];
        (*p_scaled)[i].x1 = ((*p_scaled)[i].x1 * width);
        (*p_scaled)[i].x2 = ((*p_scaled)[i].x2 * width);
        (*p_scaled)[i].y1 = height -((*p_scaled)[i].y1 * height);
        (*p_scaled)[i].y2 = height -((*p_scaled)[i].y2 * height);
    }

    return p_scaled;
}