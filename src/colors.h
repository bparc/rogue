fn inline v4 Black(void)
{
	return RGB(22, 22, 22);
}

fn inline v4 DarkBlue(void)
{
	return RGB(29, 43, 83);
}

fn inline v4 DarkPurple(void)
{
	return RGB(126, 37, 83);
}

fn inline v4 DarkGreen(void)
{
	return RGB(0, 135, 81);
}

fn inline v4 Brown(void)
{
	return RGB(171, 82, 54);
}

fn inline v4 DarkGrey(void)
{
	return RGB(95, 87, 79);
}

fn inline v4 LightGrey(void)
{
	return RGB(194, 195, 199);
}

fn inline v4 PureWhite(void)
{
	return RGB(255, 255, 255);
}

fn inline v4 White(void)
{
	return RGB(255, 241, 232);
}

fn inline v4 Red(void)
{
	return RGB(255, 0, 77);
}

fn inline v4 Orange(void)
{
	return RGB(255, 163, 0);
}

fn inline v4 Yellow(void)
{
	return RGB(255, 236, 39);
}

fn inline v4 Green(void)
{
	return RGB(0, 228, 54);
}

fn inline v4 Blue(void)
{
	return RGB(41, 173, 255);
}

fn inline v4 Lavender(void)
{
	return RGB(255, 119, 168);
}

fn inline v4 Pink(void)
{
	return RGB(255, 119, 168);
}

fn inline v4 LightPeach(void)
{
	return RGB(255, 204, 170);
}

fn inline v4 ColorFromPalette(s32 index)
{
	const v4 colors[8] = {Red(), Green(), Pink(), Blue()};
	return colors[index % ArraySize(colors)];
}