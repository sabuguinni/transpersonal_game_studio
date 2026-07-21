#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Arch_WaterIntegratedShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_WaterShelterType : uint8
{
    StreamsideLeanTo     UMETA(DisplayName = "Streamside Lean-To"),
    PondRockShelter      UMETA(DisplayName = "Pond Rock Shelter"),
    WaterfallCave        UMETA(DisplayName = "Waterfall Cave"),
    MarshlandHut         UMETA(DisplayName = "Marshland Hut"),
    RiverbankBurrow      UMETA(DisplayName = "Riverbank Burrow"),
    SpringSideDwelling   UMETA(DisplayName = "Spring-Side Dwelling")
};

USTRUCT(BlueprintType)
struct FArch_WaterShelterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    EArch_WaterShelterType ShelterType = EArch_WaterShelterType::StreamsideLeanTo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    float WaterProximity = 500.0f; // Distance to water source

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    float ShelterCapacity = 2.0f; // Number of occupants

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    bool bHasWaterAccess = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    float MoistureLevel = 0.6f; // 0.0 = dry, 1.0 = very humid

    FArch_WaterShelterConfig()
    {
        ShelterType = EArch_WaterShelterType::StreamsideLeanTo;
        WaterProximity = 500.0f;
        ShelterCapacity = 2.0f;
        StructuralIntegrity = 100.0f;
        bHasFirePit = true;
        bHasWaterAccess = true;
        MoistureLevel = 0.6f;
    }
};

USTRUCT(BlueprintType)
struct FArch_ShelterArtifacts
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifacts")
    TArray<FString> ToolMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifacts")
    TArray<FString> StoredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifacts")
    int32 FireUseCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifacts")
    float LastOccupiedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifacts")
    bool bShowsRecentActivity = false;

    FArch_ShelterArtifacts()
    {
        ToolMarks.Empty();
        StoredItems.Empty();
        FireUseCount = 0;
        LastOccupiedTime = 0.0f;
        bShowsRecentActivity = false;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_WaterIntegratedShelterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_WaterIntegratedShelterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core shelter configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter System")
    FArch_WaterShelterConfig ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter System")
    FArch_ShelterArtifacts ShelterArtifacts;

    // Water integration properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Integration")
    float WaterAccessRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Integration")
    bool bNearRunningWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Integration")
    bool bNearStillWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Integration")
    float WaterSoundIntensity = 0.5f;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float HumidityLevel = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float TemperatureModifier = 0.0f; // Cooling effect from water

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    bool bHasMistEffect = false;

    // Structural components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* StorageMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* MistParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* FireParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WaterAmbientSound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* FireCrackleSound;

    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "Water Shelter System")
    void InitializeShelter(EArch_WaterShelterType InShelterType, FVector WaterSourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Water Shelter System")
    void UpdateWaterProximity(FVector WaterSourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Water Shelter System")
    void SetShelterOccupancy(int32 OccupantCount);

    UFUNCTION(BlueprintCallable, Category = "Water Shelter System")
    void ActivateFirePit(bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Water Shelter System")
    void AddArtifact(const FString& ArtifactDescription);

    UFUNCTION(BlueprintCallable, Category = "Water Shelter System")
    void UpdateStructuralIntegrity(float IntegrityChange);

    UFUNCTION(BlueprintPure, Category = "Water Shelter System")
    float GetWaterAccessScore() const;

    UFUNCTION(BlueprintPure, Category = "Water Shelter System")
    bool IsNearWaterSource() const;

    UFUNCTION(BlueprintPure, Category = "Water Shelter System")
    FString GetShelterDescription() const;

private:
    void UpdateEnvironmentalEffects();
    void UpdateMistEffect();
    void UpdateAudioEffects();
    void ConfigureShelterByType();
    void SpawnShelterArtifacts();
    void UpdateWaterSoundIntensity();

    // Internal state
    float LastUpdateTime;
    bool bIsInitialized;
    FVector NearestWaterSource;
    float DistanceToWater;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AArch_WaterIntegratedShelterActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_WaterIntegratedShelterActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UArch_WaterIntegratedShelterSystem* ShelterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    EArch_WaterShelterType InitialShelterType = EArch_WaterShelterType::StreamsideLeanTo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Shelter")
    FVector WaterSourceLocation = FVector::ZeroVector;

    UFUNCTION(BlueprintCallable, Category = "Water Shelter")
    void InitializeWithWaterSource(FVector InWaterSourceLocation);
};