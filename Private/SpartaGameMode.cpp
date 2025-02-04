// Fill out your copyright notice in the Description page of Project Settings.
#include "SpartaGameMode.h"
#include "SpartaPawn.h"
#include "SpartaPlayerController.h"

ASpartaGameMode::ASpartaGameMode()
{
	DefaultPawnClass = ASpartaPawn::StaticClass();
	PlayerControllerClass = ASpartaPlayerController::StaticClass();

	UE_LOG(LogTemp, Warning, TEXT("SpartaGameMode"));
}