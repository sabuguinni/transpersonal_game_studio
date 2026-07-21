#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Arch_CretaceousInteriorSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorArtifact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FString ArtifactName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FVector RelativeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    TSoftObjectPtr<UStaticMesh> ArtifactMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    TSoftObjectPtr<UMaterialInterface> ArtifactMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    float WearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    bool bIsInteractable;

    FArch_InteriorArtifact()
    {
        ArtifactName = TEXT("Unknown Artifact");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WearLevel = 0.5f;
        bIsInteractable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FString LayoutName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TArray<FArch_InteriorArtifact> Artifacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector CenterPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float LayoutRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float AmbientLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FLinearColor AmbientLightColor;

    FArch_InteriorLayout()
    {
        LayoutName = TEXT("Basic Interior");
        CenterPoint = FVector::ZeroVector;
        LayoutRadius = 500.0f;
        AssociatedBiome = EBiomeType::Forest;
        AmbientLightIntensity = 100.0f;
        AmbientLightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    }
};

UCLASS(ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_CretaceousInteriorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_CretaceousInteriorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === INTERIOR GENERATION ===
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void GenerateInteriorLayout(const FArch_InteriorLayout& LayoutConfig);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void PlaceArtifact(const FArch_InteriorArtifact& Artifact);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void SetupInteriorLighting(float Intensity, const FLinearColor& Color);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void AddInteriorAmbience(TSoftObjectPtr<USoundCue> AmbientSound);

    // === STORYTELLING THROUGH ARTIFACTS ===
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateHabitationStory(const FString& StoryTheme);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddWearPatterns(float WearIntensity);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlacePersonalBelongings(int32 InhabitantCount);

    // === ATMOSPHERIC ENHANCEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableFireEffects(const FVector& FireLocation);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void AddDustParticles(float Density);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupMoodLighting(const FString& MoodType);

    // === BIOME-SPECIFIC INTERIORS ===
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AdaptToForestCave();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AdaptToMountainShelter();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AdaptToSwampDwelling();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AdaptToDesertHideout();

protected:
    // === CORE PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_InteriorLayout> AvailableLayouts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System", meta = (AllowPrivateAccess = "true"))
    FArch_InteriorLayout CurrentLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> SpawnedArtifacts;

    // === LIGHTING COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    TArray<UPointLightComponent*> InteriorLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    float BaseLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    FLinearColor WarmLightColor;

    // === AUDIO COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<USoundCue> CaveAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<USoundCue> FireCrackleSound;

    // === PARTICLE EFFECTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    TArray<UParticleSystemComponent*> EnvironmentalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UParticleSystem> FireParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UParticleSystem> DustParticleSystem;

    // === STORYTELLING DATA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    TArray<FString> HabitationStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    float InteriorAge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    int32 FormerInhabitantCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    bool bShowsSignsOfStruggle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    bool bHasPersonalTouches;

private:
    // === INTERNAL HELPERS ===
    void InitializeDefaultLayouts();
    void SpawnArtifactActor(const FArch_InteriorArtifact& Artifact);
    void ConfigureLightingMood(const FString& MoodType);
    void AddEnvironmentalParticles(const FString& EffectType, const FVector& Location);
    FArch_InteriorLayout GenerateLayoutForBiome(EBiomeType BiomeType);
};