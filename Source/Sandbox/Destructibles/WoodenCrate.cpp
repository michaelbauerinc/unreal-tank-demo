#include "WoodenCrate.h"

AWoodenCrate::AWoodenCrate()
{
	// Crate properties - uses default cube mesh from base class
	MaxHealth = 30.f;  // Weak
	DebrisCount = 5;
	DebrisScale = 0.35f;
	DebrisForce = 600.f;
	DebrisColor = FLinearColor(0.55f, 0.35f, 0.15f);  // Wood brown

	SetActorScale3D(FVector(1.f, 1.f, 1.f));
}
