#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_StructuralSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    Pillar          UMETA(DisplayName = "Stone Pillar"),
    Archway         UMETA(DisplayName = "Stone Archway"),
    Platform        UMETA(DisplayName = "Stone Platform"),
    Ruin            UMETA(DisplayName = "Ancient Ruin"),
    Monument        UMETA(DisplayName = "Monument"),
    Foundation      UMETA(DisplayName = "Foundation")
};

UENUM(BlueprintType)
enum class EArch_WeatherState : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Overgrown       UMETA(DisplayName = "Overgrown"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatherState WeatherState = EArch_WeatherState::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float Age = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsDiscovered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString HistoricalContext;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        WeatherState = EArch_WeatherState::Weathered;
        StructuralIntegrity = 0.75f;
        Age = 1000.0f;
        bIsDiscovered = false;
        HistoricalContext = TEXT("Ancient structure from unknown civilization");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructuralSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructuralSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* DiscoveryTrigger;

    // Structure Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> StructureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> WeatherMaterials;

    // Discovery System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    float DiscoveryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    bool bRequiresExamination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    FString DiscoveryText;

    // Environmental Storytelling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> EnvironmentalClues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasHiddenSecrets = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector SecretLocation;

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType Type, EArch_WeatherState Weather);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateWeatherState(EArch_WeatherState NewState);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void TriggerDiscovery(AActor* DiscoveringActor);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RevealEnvironmentalClues();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureDiscovered(AActor* DiscoveringActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureExamined(AActor* ExaminingActor);

    // Overlap Events
    UFUNCTION()
    void OnDiscoveryTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void SetupStructureMesh();
    void ApplyWeatherEffects();
    void ConfigureDiscoverySystem();
};