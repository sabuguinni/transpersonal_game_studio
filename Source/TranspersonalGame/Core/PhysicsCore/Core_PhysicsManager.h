#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Static          UMETA(DisplayName = "Static"),
    Kinematic       UMETA(DisplayName = "Kinematic"),
    Simulated       UMETA(DisplayName = "Simulated"),
    Ragdoll         UMETA(DisplayName = "Ragdoll")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Static;

    FCore_PhysicsSettings()
    {
        Mass = 100.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        PhysicsMode = ECore_PhysicsMode::Static;
    }
};

USTRUCT(BlueprintType)
struct FCore_CollisionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionObjectType> ObjectType = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionResponse> CollisionResponse = ECR_Block;

    FCore_CollisionSettings()
    {
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        ObjectType = ECC_WorldStatic;
        CollisionResponse = ECR_Block;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsManagerComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    FCore_CollisionSettings CollisionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bCanEnterRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollForceThreshold = 1000.0f;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DisableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyImpulse(FVector Impulse, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetMass(float NewMass);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetMass() const;

private:
    UPROPERTY()
    UStaticMeshComponent* StaticMeshComp;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    bool bIsRagdollActive = false;

    void FindMeshComponents();
    void ApplySettingsToStaticMesh();
    void ApplySettingsToSkeletalMesh();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCore_PhysicsManagerComponent* PhysicsManagerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Physics")
    float GlobalGravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Physics")
    float GlobalPhysicsTimestep = 0.016667f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Physics")
    int32 MaxPhysicsSteps = 8;

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void ApplyGlobalPhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void UnregisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    TArray<AActor*> GetAllPhysicsActors() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void EnableRagdollForAllCharacters();

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void DisableRagdollForAllCharacters();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidatePhysicsSetup();

private:
    UPROPERTY()
    TArray<AActor*> RegisteredPhysicsActors;

    void UpdateGlobalPhysicsSettings();
};