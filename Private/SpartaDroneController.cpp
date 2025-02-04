// Fill out your copyright notice in the Description page of Project Settings.

#include "SpartaDroneController.h"

#include "EnhancedInputSubsystems.h"

ASpartaDroneController::ASpartaDroneController()
	:InputMappingContext(nullptr),
	MoveUpAction(nullptr),
	MoveForwardAction(nullptr),
	MoveRightAction(nullptr),
	LookPitchAction(nullptr),
	LookRollAction(nullptr),
	LookYawAction(nullptr)
{}

void ASpartaDroneController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				SubSystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}