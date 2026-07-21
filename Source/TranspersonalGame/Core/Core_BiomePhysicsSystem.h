#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_BiomePhysicsSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_BiomePhysicsType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FCore_BiomePhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float WindResistance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GroundStability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float TemperatureEffect = 0.0f;

    FCore_BiomePhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        WindResistance = 0.1f;
        GroundStability = 1.0f;
        TemperatureEffect = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_BiomePhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_BiomePhysicsSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome physics configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Physics")
    ECore_BiomePhysicsType CurrentBiome = ECore_BiomePhysicsType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Physics")
    TMap<ECore_BiomePhysicsType, FCore_BiomePhysicsProperties> BiomeProperties;

    // Physics application methods
    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void SetBiomePhysics(ECore_BiomePhysicsType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    FCore_BiomePhysicsProperties GetCurrentBiomeProperties() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void ApplyBiomePhysicsToActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void UpdatePhysicsForLocation(const FVector& WorldLocation);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyWindEffect(const FVector& WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyTemperatureEffect(float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyMoistureEffect(float MoistureLevel);

    // Terrain interaction
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateGroundFriction(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateGroundStability(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationInWater(const FVector& Location) const;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxUpdateDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDetailedPhysics = true;

private:
    // Internal state
    float LastUpdateTime = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;
    
    // Biome detection
    ECore_BiomePhysicsType DetectBiomeAtLocation(const FVector& Location) const;
    
    // Physics calculation helpers
    float CalculateBiomeTransition(const FVector& Location, ECore_BiomePhysicsType& OutPrimaryBiome, ECore_BiomePhysicsType& OutSecondaryBiome, float& OutBlendFactor) const;
    
    // Initialize default biome properties
    void InitializeBiomeProperties();
    
    // Performance tracking
    int32 PhysicsUpdatesThisFrame = 0;
    float FrameStartTime = 0.0f;
};