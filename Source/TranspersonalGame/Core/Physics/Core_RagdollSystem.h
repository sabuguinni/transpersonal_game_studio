#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/Shared/SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ImpactForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bUsePhysicsBlend = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FVector HitDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FName HitBoneName = NAME_None;

    FCore_RagdollData()
    {
        ImpactForce = 1000.0f;
        BlendTime = 0.5f;
        bUsePhysicsBlend = true;
        HitDirection = FVector::ZeroVector;
        HitBoneName = NAME_None;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FCore_RagdollData& RagdollData);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollPhysicsBlend(float BlendWeight);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float PhysicsBlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    bool bAutoDeactivate = true;

private:
    bool bIsRagdollActive = false;
    float RagdollTimer = 0.0f;
    FCore_RagdollData CurrentRagdollData;

    void FindSkeletalMeshComponent();
    void BlendToRagdoll();
    void BlendFromRagdoll();
};

UCLASS()
class TRANSPERSONALGAME_API UCore_RagdollManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    static UCore_RagdollManager* GetRagdollManager(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void RegisterRagdollComponent(UCore_RagdollComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void UnregisterRagdollComponent(UCore_RagdollComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void DeactivateAllRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    int32 GetActiveRagdollCount() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Manager")
    void SetGlobalRagdollSettings(float MaxTime, bool bAutoDeactivate);

protected:
    UPROPERTY()
    TArray<UCore_RagdollComponent*> ActiveRagdolls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalMaxRagdollTime = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bGlobalAutoDeactivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    int32 MaxSimultaneousRagdolls = 20;

private:
    void CleanupInvalidComponents();
};