#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Arch_PrimitiveStructureSystem.h"
#include "SharedTypes.h"
#include "Arch_ShelterEntranceActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterEntranceActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterEntranceActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UArch_PrimitiveStructureSystem* StructureSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float ShelterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float TemperatureBonus = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bProvidesWindProtection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bProvidesRainProtection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowDebugRadius = false;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsPlayerInShelter(AActor* PlayerActor) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetShelterEffectiveness() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    FVector GetOptimalShelterPosition() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerEnterShelter(AActor* PlayerActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerExitShelter(AActor* PlayerActor);

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    TArray<AActor*> ActorsInShelter;
    
    void UpdateShelterEffects();
    void DrawDebugShelterRadius();
};