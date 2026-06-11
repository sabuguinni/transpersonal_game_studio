#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "EnvironmentalStorytellingManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_StoryElementType : uint8
{
    FallenLog           UMETA(DisplayName = "Fallen Log"),
    AbandonedCamp       UMETA(DisplayName = "Abandoned Camp"),
    BoneScatter         UMETA(DisplayName = "Bone Scatter"),
    WeatheredRock       UMETA(DisplayName = "Weathered Rock"),
    AncientCarving      UMETA(DisplayName = "Ancient Carving"),
    FloodDebris         UMETA(DisplayName = "Flood Debris")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_StoryElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    EEnvArt_StoryElementType ElementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    FString StoryContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    bool bHasAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Element")
    float InteractionRadius;

    FEnvArt_StoryElement()
    {
        ElementType = EEnvArt_StoryElementType::FallenLog;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        StoryContext = TEXT("An untold story from the past...");
        bHasAmbientSound = false;
        InteractionRadius = 200.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float VolumetricFogIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    bool bEnableDustMotes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    int32 DustMoteCount;

    FEnvArt_AtmosphericSettings()
    {
        SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);  // Golden hour
        SunIntensity = 8.0f;
        SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
        FogDensity = 0.6f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        VolumetricFogIntensity = 1.2f;
        bEnableDustMotes = true;
        DustMoteCount = 50;
    }
};

/**
 * Manages environmental storytelling elements and atmospheric enhancement
 * Creates immersive prehistoric environments that tell stories without words
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentalStorytellingManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentalStorytellingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Elements")
    TArray<FEnvArt_StoryElement> StoryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateStoryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxStoryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GenerationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EEnvArt_StoryElementType, class USoundCue*> ElementAmbientSounds;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void GenerateStoryElements();

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void PlaceStoryElement(const FEnvArt_StoryElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void ClearAllStoryElements();

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    TArray<FEnvArt_StoryElement> GetNearbyStoryElements(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void ApplyAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void EnableGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void CreateDustMoteEffects();

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Storytelling")
    void OnStoryElementDiscovered(const FEnvArt_StoryElement& Element);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Storytelling")
    void OnAtmosphericChange(const FEnvArt_AtmosphericSettings& NewSettings);

protected:
    UFUNCTION()
    void OnStoryElementTriggerEnter(class UPrimitiveComponent* OverlappedComponent, 
                                   class AActor* OtherActor, 
                                   class UPrimitiveComponent* OtherComponent, 
                                   int32 OtherBodyIndex, 
                                   bool bFromSweep, 
                                   const FHitResult& SweepResult);

private:
    UPROPERTY()
    TArray<class AActor*> SpawnedStoryActors;

    UPROPERTY()
    class ADirectionalLight* SceneSun;

    UPROPERTY()
    class AAtmosphericFog* SceneFog;

    UPROPERTY()
    TArray<class UBoxComponent*> StoryTriggers;

    void FindOrCreateSun();
    void FindOrCreateFog();
    void UpdateLightingForTimeOfDay(float TimeOfDay);
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;
    EEnvArt_StoryElementType GetRandomStoryElementType() const;
    void CreateStoryElementMesh(const FEnvArt_StoryElement& Element, AActor* SpawnedActor);
};

#include "EnvironmentalStorytellingManager.generated.h"