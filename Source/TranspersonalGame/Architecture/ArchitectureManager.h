#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "ArchitectureManager.generated.h"

USTRUCT(BlueprintType)
struct FArchitecturalElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConsciousnessResponsive;

    FArchitecturalElement()
    {
        ElementName = TEXT("DefaultElement");
        MaterialType = TEXT("consciousness_crystal");
        Dimensions = FVector(100.0f, 100.0f, 100.0f);
        ConsciousnessResonance = 1.0f;
        bIsConsciousnessResponsive = true;
    }
};

USTRUCT(BlueprintType)
struct FSacredSpace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FArchitecturalElement> Elements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessAmplification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxOccupancy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredConsciousnessLevel;

    FSacredSpace()
    {
        SpaceName = TEXT("DefaultSacredSpace");
        SpaceType = TEXT("meditation_sanctuary");
        ConsciousnessAmplification = 1.5f;
        MaxOccupancy = 8;
        RequiredConsciousnessLevel = TEXT("contemplative");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Spaces")
    TArray<FSacredSpace> SacredSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArchitecturalElement> AvailableElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float BaseConsciousnessResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<FString, float> MaterialConsciousnessMultipliers;

    // Reference to consciousness system
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class AConsciousnessSystem* ConsciousnessSystemRef;

public:
    virtual void Tick(float DeltaTime) override;

    // Sacred space management
    UFUNCTION(BlueprintCallable, Category = "Sacred Spaces")
    void CreateSacredSpace(const FSacredSpace& SpaceDefinition, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Sacred Spaces")
    FSacredSpace GetSacredSpaceByName(const FString& SpaceName);

    UFUNCTION(BlueprintCallable, Category = "Sacred Spaces")
    TArray<FSacredSpace> GetSacredSpacesByType(const FString& SpaceType);

    // Architectural element management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PlaceArchitecturalElement(const FArchitecturalElement& Element, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateElementConsciousnessResonance(const FString& ElementName, float NewResonance);

    // Consciousness interaction
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    float GetSpaceConsciousnessAmplification(const FString& SpaceName);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void RespondToConsciousnessLevel(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool CanAccessSpace(const FString& SpaceName, float PlayerConsciousnessLevel);

    // Material system integration
    UFUNCTION(BlueprintCallable, Category = "Materials")
    float GetMaterialConsciousnessMultiplier(const FString& MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateMaterialProperties(const FString& MaterialType, float ConsciousnessLevel);

    // Sacred geometry
    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    FVector CalculateSacredGeometryPosition(const FString& GeometryType, int32 Index, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    TArray<FVector> GenerateFlowerOfLifePattern(FVector Center, float Scale, int32 Layers);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    TArray<FVector> GenerateFibonacciSpiral(FVector StartPoint, float GrowthFactor, int32 Points);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ActivateSpaceEffects(const FString& SpaceName);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void DeactivateSpaceEffects(const FString& SpaceName);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateLightingBasedOnConsciousness(float ConsciousnessLevel);

protected:
    // Internal helper functions
    void InitializeDefaultSacredSpaces();
    void InitializeMaterialMultipliers();
    void LoadArchitecturalAssets();
    void SetupConsciousnessCallbacks();

    // Sacred geometry calculations
    FVector CalculateGoldenRatioPosition(FVector Base, float Distance, float Angle);
    TArray<FVector> GeneratePlatonicSolidVertices(const FString& SolidType, float Scale);
    
    // Dynamic architecture
    void UpdateArchitectureBasedOnConsciousness();
    void TransformMaterialsForConsciousnessLevel(float Level);
    
private:
    // Internal state
    float LastConsciousnessLevel;
    TMap<FString, bool> ActiveSpaceEffects;
    TArray<UStaticMeshComponent*> DynamicArchitecturalElements;
};