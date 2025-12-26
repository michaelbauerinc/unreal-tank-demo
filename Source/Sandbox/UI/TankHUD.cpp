#include "TankHUD.h"
#include "Engine/Canvas.h"

void ATankHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.4f;  // 40% from top (higher on screen)

	FLinearColor Green(0.2f, 1.f, 0.2f, 0.9f);
	float Size = 20.f;
	float Gap = 6.f;
	float Thick = 2.f;

	// Crosshair lines
	DrawLine(CX - Size, CY, CX - Gap, CY, Green, Thick);
	DrawLine(CX + Gap, CY, CX + Size, CY, Green, Thick);
	DrawLine(CX, CY - Size, CX, CY - Gap, Green, Thick);
	DrawLine(CX, CY + Gap, CX, CY + Size, Green, Thick);

	// Center dot
	DrawRect(Green, CX - 2.f, CY - 2.f, 4.f, 4.f);
}
