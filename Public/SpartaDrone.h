// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpartaDrone.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCapsuleComponent;

struct FInputActionValue;

UCLASS()
class ASSIGNMENT_7_7_API ASpartaDrone : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASpartaDrone();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
    float MaxDroneEnginePower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
    float DroneEnginePower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
    float GravityAccel;

protected:
    FVector CumulativeUpOffset = FVector::ZeroVector;

	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void MoveUp(const FInputActionValue& value);
	UFUNCTION()
	void MoveForward(const FInputActionValue& value);
    UFUNCTION()
    void MoveRight(const FInputActionValue& value);
    UFUNCTION()
    void LookPitch(const FInputActionValue& value);
    UFUNCTION()
    void LookRoll(const FInputActionValue& value);
    UFUNCTION()
    void LookYaw(const FInputActionValue& value);

private:	
    FRotator TargetRotation;

    void SetGravity(float DeltaTime);
    void ReduceEnginePower(float DeltaTime);
    void UpdateCamera(float DeltaTime);
    void ApplyTiltEffect(float DeltaTime);
    void RestoreTilt(float DeltaTime);
    bool IsGrounded();

    float MinYaw = -360.0f;
    float MaxYaw = 360.0f;

    float MinPitch = -45.0f;
    float MaxPitch = 45.0f;

    float YawSpeed = 50.0f;
    float PitchSpeed = 50.0f;

    float ReducingPower = 100.0f;

    float CurrentMoveAxisValue;
    float CurrentMoveForwardAxis;

    bool bIsGrounded = false;
    FRotator AccumulatedRotation;
};
