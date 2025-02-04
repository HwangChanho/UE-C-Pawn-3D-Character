// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpartaPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCapsuleComponent;

//class USkeletalMeshComponent;

struct FInputActionValue;

UCLASS()
class ASSIGNMENT_7_7_API ASpartaPawn : public APawn
{
	GENERATED_BODY()

public:
	ASpartaPawn();

    /** Collision Component (Capsule, Root) */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UCapsuleComponent* CapsuleComp;

    /** Mesh Component */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComp;

    /** Spring Arm Component */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USpringArmComponent* SpringArmComp;

    /** Camera Component */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UCameraComponent* CameraComp;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Startjump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);
	
private:
	float CurrentSpeed;
	float SprintSpeed;
	float WalkingSpeed;

	float CurrentFloorZ;
	float Gravity;
	bool bIsJumping;
	bool bIsSprinting;
	FVector Velocity;

	FVector LastLocation;
	bool bIsMoving;

	// Collision
	float CapsuleRadius;
	float CapsuleHalfHeight;
	FVector BlockedPosition;
	TSet<AActor*> OverlappingActors;

	void UpdateFloorZ();
	void MovementByActorWorldOffset(const FVector2D moveInput);
	void SetActorRotate(FVector MoveDirection);

	// 충돌 관련
	// void OnCustomCollision(AActor* OtherActor, const FHitResult& HitResult);
	bool CheckCollision();
};
