#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_VolcanicCaveSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_CaveType : uint8
{
    LavaTube        UMETA(DisplayName = "Lava Tube"),
    ThermalCave     UMETA(DisplayName = "Thermal Cave"),
    CoolingChamber  UMETA(DisplayName = "Cooling Chamber"),
    ObsidianCave    UMETA(DisplayName = "Obsidian Cave"),
    SteamVent       UMETA(DisplayName = "Steam Vent Cave")
};

UENUM(BlueprintType)
enum class EArch_ThermalLevel : uint8
{
    Cool        UMETA(DisplayName = "Cool (20-30°C)"),
    Warm        UMETA(DisplayName = "Warm (30-50°C)"),
    Hot         UMETA(DisplayName = "Hot (50-80°C)"),
    Scalding    UMETA(DisplayName = "Scalding (80-100°C)"),
    Lethal      UMETA(DisplayName = "Lethal (100°C+)")
};

USTRUCT(BlueprintType)
struct FArch_CaveProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    EArch_CaveType CaveType = EArch_CaveType::LavaTube;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    EArch_ThermalLevel ThermalLevel = EArch_ThermalLevel::Warm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float CaveDepth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float EntranceWidth = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float ThermalRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasMineralDeposits = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float OxygenLevel = 0.8f;

    FArch_CaveProperties()
    {
        CaveType = EArch_CaveType::LavaTube;
        ThermalLevel = EArch_ThermalLevel::Warm;
        CaveDepth = 500.0f;
        EntranceWidth = 300.0f;
        ThermalRadius = 1000.0f;
        bHasWaterSource = false;
        bHasMineralDeposits = true;
        OxygenLevel = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_VolcanicCaveSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_VolcanicCaveSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveEntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* ThermalZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteriorZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SteamParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* AshParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ThermalAmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* CaveEchoAudio;

    // Cave Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    FArch_CaveProperties CaveProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float CurrentTemperature = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float HumidityLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    bool bIsActive = true;

    // Interior Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<AActor*> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    int32 MaxOccupants = 4;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    TArray<AActor*> CurrentOccupants;

    // Lighting
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UPointLightComponent* ThermalGlow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USpotLightComponent* EntranceLight;

public:
    // Cave System Functions
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetCaveType(EArch_CaveType NewType);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetThermalLevel(EArch_ThermalLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void UpdateThermalEffects();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetTemperatureAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool IsLocationSafe(FVector Location) const;

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool CanEnterCave(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void EnterCave(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ExitCave(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SpawnInteriorProps();

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateAudioEffects();

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateLighting();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Cave System")
    FString GetCaveDescription() const;

    UFUNCTION(BlueprintPure, Category = "Cave System")
    bool HasWaterSource() const { return CaveProperties.bHasWaterSource; }

    UFUNCTION(BlueprintPure, Category = "Cave System")
    bool HasMineralDeposits() const { return CaveProperties.bHasMineralDeposits; }

protected:
    // Internal Functions
    void InitializeCaveComponents();
    void SetupThermalZone();
    void ConfigureLighting();
    void UpdateTemperatureGradient();
    float CalculateThermalDamage(float Temperature) const;

    // Overlap Events
    UFUNCTION()
    void OnThermalZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnThermalZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnInteriorZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // Internal State
    float ThermalUpdateTimer = 0.0f;
    float ParticleUpdateTimer = 0.0f;
    float AudioUpdateTimer = 0.0f;
    
    // Constants
    static constexpr float THERMAL_UPDATE_INTERVAL = 1.0f;
    static constexpr float PARTICLE_UPDATE_INTERVAL = 0.5f;
    static constexpr float AUDIO_UPDATE_INTERVAL = 2.0f;
    static constexpr float SAFE_TEMPERATURE_THRESHOLD = 60.0f;
    static constexpr float LETHAL_TEMPERATURE_THRESHOLD = 100.0f;
};