void render_32bit_score(UINT32 score, UINT8 *buffer) {  // THIS FUNCTION IS A CUSTOM SPRINTF BECAUSE SPRINTF ONLY SUPPORTS UP TO 16 BIT SON GB. THIS FUNC ALLOWS US TO USE 32BIT AND MANUALLY MANIPULATE THE ARRAY
    UINT8 *old_buffer = buffer;                         // buffer = score_buffer[0]
    while (score > 0) {
        // dereference buffer[0] (*), change its value, THEN ++ to buffer[1] (++buffer vs buffer++) pre-increment vs post-increment
        // score % 10 gets me the farthest right digit, then dividing by 10 removes that right digit, which brings me to the next digit to its left
        // By adding '0', we convert the integer into an ASCII code value. So: 5 + '0' = '5'
        *buffer++ = (score % 10) + '0';
        score /= 10;  // shifts out the rightmost digit
    }
    // REMEMBER bugger is actually score_buffer[?] because of previous buffer++! Hence why we need old_buffer
    *buffer = 0;          // when the while loop runs out of digits to add to the buffer, change the next character in the array to equal 0. This is check inside of update_score()'s for loop.
    reverse(old_buffer);  // reverses the pointer's contents until it reaches a 0 or NULL in the array, so that the score displays correctly
}
// if (score > 65535) {
//     render_32bit_score(score, score_buffer);
// } else
// sprintf...