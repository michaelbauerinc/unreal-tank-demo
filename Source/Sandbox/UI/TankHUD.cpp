#include "TankHUD.h"
#include "Engine/Canvas.h"

void ATankHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	// Fixed crosshair at screen center
	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;

	DrawCrosshairAt(CX, CY);
}

void ATankHUD::DrawCrosshairAt(float CX, float CY)
{
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
