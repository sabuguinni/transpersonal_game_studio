#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_StoneCircleMonument.generated.h"

UENUM(BlueprintType)
enum class EArch_StoneCircleType : uint8
{
    Ancient_Megalith    UMETA(DisplayName = "Ancient Megalith"),
    Weathered_Sandstone UMETA(DisplayName = "Weathered Sandstone"),
    Volcanic_Basalt     UMETA(DisplayName = "Volcanic Basalt"),
    Limestone_Carved    UMETA(DisplayName = "Limestone Carved"),
    Granite_Monolith    UMETA(DisplayName = "Granite Monolith")
};

USTRUCT(BlueprintType)
struct FArch_StoneProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Properties")
    float WeatheringLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Properties")
    float MossGrowth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Properties")
    float StructuralIntegrity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Properties")
    bool bHasAncientCarvings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Properties")
    float AgeInYears = 10000.0f;

    FArch_StoneProperties()
    {
        WeatheringLevel = 0.7f;
        MossGrowth = 0.5f;
        StructuralIntegrity = 0.8f;
        bHasAncientCarvings = true;
        AgeInYears = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StoneCircleMonument : public AActor
{
    GENERATED_BODY()

public:
    AArch_StoneCircleMonument();

protected:
    virtual void BeginPlay() override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    // Stone Circle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Circle")
    EArch_StoneCircleType CircleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Circle")
    int32 NumberOfStones = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Circle")
    float CircleRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Circle")
    float StoneHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone Circle")
    FArch_StoneProperties StoneProperties;

    // Individual Stone Meshes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UStaticMeshComponent*> StoneMeshes;

    // Environmental Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableWindEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientTemperatureModifier = -2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float HumidityLevel = 0.6f;

    // Interaction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBeExamined = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bProvidesShade = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float ShadeRadius = 800.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // Stone Circle Management
    UFUNCTION(BlueprintCallable, Category = "Stone Circle")
    void GenerateStoneCircle();

    UFUNCTION(BlueprintCallable, Category = "Stone Circle")
    void SetCircleType(EArch_StoneCircleType NewType);

    UFUNCTION(BlueprintCallable, Category = "Stone Circle")
    void UpdateStoneProperties(const FArch_StoneProperties& NewProperties);

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsPlayerInShade(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FString GetMonumentDescription() const;

    // Player Interaction
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerEnterCircle(class APawn* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerExitCircle(class APawn* Player);

protected:
    // Internal Methods
    void CreateStoneAtPosition(const FVector& Position, const FRotator& Rotation, int32 StoneIndex);
    void ApplyWeatheringEffects();
    void UpdateEnvironmentalEffects();

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    // Internal State
    bool bIsGenerated = false;
    float LastWeatherUpdate = 0.0f;
    TArray<FVector> StonePositions;
    TArray<FRotator> StoneRotations;
};