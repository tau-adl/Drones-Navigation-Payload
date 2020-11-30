/*
 * img_file.h
 *
 *  Created on: Mar 21, 2020
 *      Author: Dan
 */

/*
 *
 *
 * a python generated output for the ìmage05.jpg` file (bee)
 *
	input_file = open(INPUT_FILE, "rb")

	input_file_content = input_file.read()


	output = ",0x".join("{:02x}".format(ord(c)) for c in input_file_content)

	outputFile = open(OUTPUT_FILE, "w")
	outputFile.write(output)
	outputFile.close()
	=================== python 3 code ===========================
	def main():
    input_file = open(INPUT_FILE, "rb")

    input_file_content = input_file.read()
    a = input_file_content[0]
    output = ", ".join(hex(c) for c in input_file_content)

    outputFile = open(OUTPUT_FILE, "w")
    outputFile.write(output)
    outputFile.close()


if __name__ == "__main__":
    main()
 *
 */

#ifndef IMG_JPG_FILE_H
#define IMG_JPG_FILE_H

typedef enum eImgStates_name
{
	eImgStates_start		= 0,
	eImgStates_sending,
	eImgStates_finished,
}eImgStates;

typedef struct stImg_name {
   uint8_t* 	m_pImg;
   uint16_t 	m_SizeB;
   uint32_t		m_SysTick;
   eImgStates	m_eCurrImgStates;
   eImgStates	m_eNextImgStates;
} stImg;


uint8_t* 	Img_jpg_GetArrPtr(void);
eImgStates  Img_jpg_GetCurrImgState(void);
eImgStates  Img_jpg_GetNextImgState(void);
eImgStates  Img_jpg_UpdateImgState(void);
stImg* 		Img_jpg_GetStruct(void);
void	 	Img_jpg_GetNewImg(void);
//stImg* 		Img_jpg_Iterate( stImg* a_pstImg);



#endif /* IMG_JPG_FILE_H */
