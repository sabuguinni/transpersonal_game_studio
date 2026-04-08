#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros Pequenos (Domesticáveis)
    Compsognathus       UMETA(DisplayName = "Compsognathus"),
    Dryosaurus         UMETA(DisplayName = "Dryosaurus"),
    Hypsilophodon      UMETA(DisplayName = "Hypsilophodon"),
    
    // Herbívoros Médios
    Parasaurolophus    UMETA(DisplayName = "Parasaurolophus"),
    Triceratops        UMETA(DisplayName = "Triceratops"),
    Stegosaurus        UMETA(DisplayName = "Stegosaurus"),
    
    // Herbívoros Grandes
    Brontosaurus       UMETA(DisplayName = "Brontosaurus"),
    Diplodocus         UMETA(DisplayName = "Diplodocus"),
    
    // Carnívoros Pequenos
    Dilophosaurus      UMETA(DisplayName = "Dilophosaurus"),
    Coelophysis        UMETA(DisplayName = "Coelophysis"),
    
    // Carnívoros Médios
    Allosaurus         UMETA(DisplayName = "Allosaurus"),
    Carnotaurus        UMETA(DisplayName = "Carnotaurus"),
    
    // Carnívoros Grandes (Apex Predators)
    TyrannosaurusRex   UMETA(DisplayName = "Tyrannosaurus Rex"),
    Giganotosaurus     UMETA(DisplayName = "Giganotosaurus"),
    Spinosaurus        UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle               UMETA(DisplayName = "Idle"),
    Foraging           UMETA(DisplayName = "Foraging"),
    Drinking           UMETA(DisplayName = "Drinking"),
    Resting            UMETA(DisplayName = "Resting"),
    Socializing        UMETA(DisplayName = "Socializing"),
    Hunting            UMETA(DisplayName = "Hunting"),
    Fleeing            UMETA(DisplayName = "Fleeing"),
    Territorial        UMETA(DisplayName = "Territorial"),
    Mating             UMETA(DisplayName = "Mating"),
    Nesting            UMETA(DisplayName = "Nesting"),
    Migrating          UMETA(DisplayName = "Migrating"),
    Investigating      UMETA(DisplayName = "Investigating"),
    Domesticated       UMETA(DisplayName = "Domesticated")
};

UENUM(BlueprintType)
enum class EDinosaurPersonality : uint8
{
    Aggressive         UMETA(DisplayName = "Aggressive"),
    Cautious           UMETA(DisplayName = "Cautious"),
    Curious            UMETA(DisplayName = "Curious"),
    Territorial        UMETA(DisplayName = "Territorial"),
    Social             UMETA(DisplayName = "Social"),
    Solitary           UMETA(DisplayName = "Solitary"),
    Protective         UMETA(DisplayName = "Protective"),
    Timid              UMETA(DisplayName = "Timid")
};

USTRUCT(BlueprintType)
struct FDinosaurNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Comfort = 100.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Localizações conhecidas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownWaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownFoodSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SafeAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerousAreas;

    // Relações sociais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> KnownActors; // Actor -> Relationship Value (-100 to 100)

    // Experiências recentes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerEncounterTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastThreatTime = -1.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurPhysicalTraits
{
    GENERATED_BODY()

    // Variações físicas únicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float SizeVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AccentColor = FLinearColor::Black;

    // Características distintivas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool HasScar = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool HasLimp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float HornSizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float TailLengthMultiplier = 1.0f;

    // Stats únicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.7", ClampMax = "1.3"))
    float SpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.7", ClampMax = "1.3"))
    float StrengthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.7", ClampMax = "1.3"))
    float IntelligenceMultiplier = 1.0f;
};

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Propriedades base do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    FString IndividualName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    EDinosaurPersonality Personality = EDinosaurPersonality::Cautious;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    FDinosaurPhysicalTraits PhysicalTraits;

    // Estado comportamental
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Behavior")
    EDinosaurBehaviorState CurrentState = EDinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FDinosaurNeeds Needs;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Behavior")
    FDinosaurMemory Memory;

    // Sistema de domesticação
    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float DomesticationLevel = 0.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool CanBeDomesticated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    AActor* BondedPlayer = nullptr;

    // Rotinas diárias
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RoutineStartTime = 6.0f; // Hora do dia (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RoutineDuration = 12.0f; // Duração em horas

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AddMemoryLocation(FVector Location, bool bIsSafe);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateActorRelationship(AActor* Actor, float RelationshipDelta);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessDomesticationInteraction(AActor* Player, float InteractionQuality);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsNocturnal() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsSocialSpecies() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsApexPredator() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    float GetThreatLevel() const;

private:
    void InitializeSpeciesTraits();
    void ProcessDailyRoutine();
    void UpdateBehaviorBasedOnNeeds();
    void ProcessMemoryDecay(float DeltaTime);
    
    // Timers
    float LastNeedsUpdate = 0.0f;
    float LastRoutineCheck = 0.0f;
    float LastMemoryUpdate = 0.0f;
};