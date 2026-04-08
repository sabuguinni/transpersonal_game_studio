#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.h"
#include "VFXComponent.generated.h"

/**
 * VFX Component - Attach to actors that need to spawn VFX
 * Provides easy interface for actors to spawn and manage their visual effects
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // VFX Spawning
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(EVFXType VFXType, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtSocket(EVFXType VFXType, FName SocketName, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFXType VFXType, FVector WorldLocation, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyAllVFX();

    // Auto VFX - Automatically spawn VFX based on events
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void EnableAutoVFX(EVFXType VFXType, float Interval = 1.0f, bool bRandomInterval = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DisableAutoVFX(EVFXType VFXType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DisableAllAutoVFX();

    // VFX Control
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXColorParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXVectorParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, FVector Vector);

    // Event-based VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void OnActorHit(AActor* HitActor, FVector HitLocation, FVector HitNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void OnActorDestroyed();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void OnMovementStateChanged(bool bIsMoving, float MovementSpeed);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EVFXType, float> AutoVFXIntervals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<EVFXType> OnHitVFXTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<EVFXType> OnDestroyVFXTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<EVFXType> OnMovementVFXTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoCleanup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float MaxVFXDistance = 2000.0f;

protected:
    // Runtime State
    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> SpawnedVFX;

    UPROPERTY()
    TMap<EVFXType, float> AutoVFXTimers;

    UPROPERTY()
    TMap<EVFXType, bool> AutoVFXEnabled;

    UPROPERTY()
    UVFXSystemManager* VFXManager;

    // Internal Methods
    void UpdateAutoVFX(float DeltaTime);
    void CleanupInvalidVFX();
    FVector GetSocketWorldLocation(FName SocketName) const;
    FRotator GetSocketWorldRotation(FName SocketName) const;
    bool IsWithinMaxDistance(const FVector& Location) const;
};