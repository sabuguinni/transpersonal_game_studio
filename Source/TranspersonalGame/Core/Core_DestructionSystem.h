#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "No Destruction"),
    Fracture        UMETA(DisplayName = "Fracture Break"),
    Shatter         UMETA(DisplayName = "Complete Shatter"),
    Crumble         UMETA(DisplayName = "Gradual Crumble"),
    Explode         UMETA(DisplayName = "Explosive Break")
};

USTRUCT(BlueprintType)
struct FCore_DestructionParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bUseGeometryCollection = true;

    FCore_DestructionParams()
    {
        ImpactThreshold = 500.0f;
        DestructionType = ECore_DestructionType::Fracture;
        MaxFragments = 20;
        FragmentLifetime = 10.0f;
        bUseGeometryCollection = true;
    }
};

/**
 * Core Destruction System Component
 * Handles realistic destruction physics for prehistoric environment objects
 * Supports Chaos Physics and Geometry Collections for authentic breaking behavior
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core destruction functionality
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void InitializeDestruction(UStaticMeshComponent* MeshComponent, const FCore_DestructionParams& Params);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void SetDestructionThreshold(float NewThreshold);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    bool CanBeDestroyed() const;

    // Geometry Collection support
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void CreateGeometryCollection(UStaticMesh* SourceMesh);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void ApplyFieldForce(const FVector& Location, float Magnitude, float Radius);

    // Fragment management
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void CleanupFragments();

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    int32 GetActiveFragmentCount() const;

protected:
    // Destruction parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionParams DestructionParams;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> TargetMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UGeometryCollectionComponent> GeometryCollectionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFieldSystemComponent> FieldSystemComponent;

    // Runtime state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    bool bIsDestroyed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    float LastImpactTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<AActor>> FragmentActors;

private:
    // Internal destruction logic
    void ExecuteFractureDestruction(const FVector& ImpactLocation, const FVector& ImpactForce);
    void ExecuteShatterDestruction(const FVector& ImpactLocation, const FVector& ImpactForce);
    void ExecuteCrumbleDestruction(const FVector& ImpactLocation, const FVector& ImpactForce);
    void ExecuteExplodeDestruction(const FVector& ImpactLocation, const FVector& ImpactForce);

    // Fragment creation and management
    AActor* CreateFragment(const FVector& Location, const FVector& Velocity, UStaticMesh* FragmentMesh);
    void ScheduleFragmentCleanup(AActor* Fragment, float Delay);

    // Chaos Physics integration
    void SetupChaosPhysics();
    void ApplyChaosForceField(const FVector& Location, float Magnitude, float Radius);

    // Timer handles
    FTimerHandle FragmentCleanupTimer;
};