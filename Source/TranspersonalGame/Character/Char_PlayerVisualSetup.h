#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_PlayerVisualSetup.generated.h"

USTRUCT(BlueprintType)
struct FChar_PlayerMeshConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> DefaultMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    FVector MeshRelativeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    FRotator MeshRelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> DefaultMaterials;

    FChar_PlayerMeshConfig()
    {
        MeshRelativeLocation = FVector(0.0f, 0.0f, -90.0f);
        MeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
    }
};

USTRUCT(BlueprintType)
struct FChar_CameraConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
    float TargetArmLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
    FVector SocketOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
    FRotator BoomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float FieldOfView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUsePawnControlRotation;

    FChar_CameraConfig()
    {
        TargetArmLength = 400.0f;
        SocketOffset = FVector(0.0f, 0.0f, 80.0f);
        BoomRotation = FRotator(-20.0f, 0.0f, 0.0f);
        FieldOfView = 90.0f;
        bUsePawnControlRotation = true;
    }
};

USTRUCT(BlueprintType)
struct FChar_CollisionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CapsuleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CapsuleHalfHeight;

    FChar_CollisionConfig()
    {
        CapsuleRadius = 42.0f;
        CapsuleHalfHeight = 96.0f;
    }
};

USTRUCT(BlueprintType)
struct FChar_MovementConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpZVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirControl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundFriction;

    FChar_MovementConfig()
    {
        MaxWalkSpeed = 600.0f;
        JumpZVelocity = 700.0f;
        AirControl = 0.35f;
        GroundFriction = 8.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisualSetup : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisualSetup();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FChar_PlayerMeshConfig MeshConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FChar_CameraConfig CameraConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FChar_CollisionConfig CollisionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FChar_MovementConfig MovementConfig;

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetupPlayerVisuals();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void ConfigureMeshComponent(USkeletalMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void ConfigureCameraSystem(USpringArmComponent* CameraBoom, UCameraComponent* FollowCamera);

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void ConfigureCollision(UCapsuleComponent* CapsuleComponent);

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void ConfigureMovement(UCharacterMovementComponent* MovementComponent);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSetup() const;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    void CacheOwnerCharacter();
};