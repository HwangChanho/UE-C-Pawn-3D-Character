// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpartaPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

DECLARE_LOG_CATEGORY_EXTERN(LogAAA, Warning, All);

/**
 * 
 */
UCLASS()
class ASSIGNMENT_7_7_API ASpartaPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ASpartaPlayerController();

	/** Mapping Context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;

	/** MoveAction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;

	/** JumpAction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* JumpAction;

	/** LookAction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;

	/** SprintAction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SprintAction;

protected:
	virtual void BeginPlay() override;
};
