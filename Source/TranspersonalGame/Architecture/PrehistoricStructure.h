#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "ArchitectureTypes.h"
#include "PrehistoricStructure.generated.h"

class UPrehistoricStructureData;

/**
 * Actor class for prehistoric structures that tell environmental stories
 * Each structure is a document of the civilization that built it
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricStructure : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricStructure();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ExteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    // Interior prop meshes that tell the story
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UStaticMeshComponent*> InteriorProps;

    // Structure data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    UPrehistoricStructureData* StructureData;

    // Dynamic properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bPlayerHasVisited;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float TimeOfLastVisit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsCurrentlyOccupiedByWildlife;

    // Environmental storytelling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowBloodStains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowToolMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowFoodRemains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> VisibleClues;

public:
    // Initialization
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeFromData(UPrehistoricStructureData* Data);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void GenerateInteriorLayout();

    // Player interaction
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerEntered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerExited();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerInvestigated();

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FString GetStructureStory() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FString> GetVisibleClues() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void RevealHiddenClue(const FString& ClueName);

    // Gameplay functions
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    bool CanProvideShel() const;

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    float GetSafetyRating() const;

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    bool ContainsUsableResources() const;

    // Wildlife interaction
    UFUNCTION(BlueprintCallable, Category = "Wildlife")
    void SetWildlifeOccupied(bool bOccupied);

    UFUNCTION(BlueprintCallable, Category = "Wildlife")
    bool IsOccupiedByWildlife() const { return bIsCurrentlyOccupiedByWildlife; }

protected:
    // Interior generation helpers
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SpawnInteriorProp(UStaticMesh* PropMesh, const FTransform& Transform, const FString& StoryContext);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ApplyWeatheringEffects();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SetupAmbientAudio();

    // Collision events
    UFUNCTION()
    void OnInteriorEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                          bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorExited(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // Internal state
    bool bIsInitialized;
    float CreationTime;

    // Story elements cache
    mutable FString CachedStoryText;
    mutable bool bStoryTextCached;
};