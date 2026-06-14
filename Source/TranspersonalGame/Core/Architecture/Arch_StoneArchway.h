#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "Arch_StoneArchway.generated.h"

UENUM(BlueprintType)
enum class EArch_ArchwayType : uint8
{
    CaveEntrance     UMETA(DisplayName = "Cave Entrance"),
    RuinedGateway    UMETA(DisplayName = "Ruined Gateway"),
    NaturalArch      UMETA(DisplayName = "Natural Stone Arch"),
    CarvedPortal     UMETA(DisplayName = "Carved Portal")
};

USTRUCT(BlueprintType)
struct FArch_ArchwayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_ArchwayType ArchwayType = EArch_ArchwayType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float Width = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float Height = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float Depth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasInteriorSpace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bBlocksWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float TemperatureModifier = 5.0f;

    FArch_ArchwayConfig()
    {
        ArchwayType = EArch_ArchwayType::CaveEntrance;
        Width = 400.0f;
        Height = 600.0f;
        Depth = 200.0f;
        bHasInteriorSpace = true;
        bBlocksWeather = true;
        TemperatureModifier = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StoneArchway : public AActor
{
    GENERATED_BODY()

public:
    AArch_StoneArchway();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* ArchwayMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UBoxComponent* EntranceTrigger;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_ArchwayConfig ArchwayConfig;

    // Interior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasSleepingArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasStorageArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> InteriorPropLocations;

    // Weather Protection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainProtectionFactor = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindProtectionFactor = 0.6f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetArchwayType(EArch_ArchwayType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsPointInsideArchway(const FVector& Point) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetWeatherProtectionAt(const FVector& Point) const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetupInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void AddInteriorProp(const FVector& Location, const FString& PropType);

    // Events
    UFUNCTION()
    void OnEntranceTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntranceTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnPlayerEnterArchway(APawn* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnPlayerExitArchway(APawn* Player);

private:
    void UpdateArchwayMesh();
    void UpdateInteriorVolume();
    void ApplyWeatherEffects();

    // Interior tracking
    UPROPERTY()
    TArray<AActor*> ActorsInside;

    // Visual effects
    UPROPERTY()
    class UNiagaraComponent* AmbientParticles;

    UPROPERTY()
    class UAudioComponent* AmbientSound;
};