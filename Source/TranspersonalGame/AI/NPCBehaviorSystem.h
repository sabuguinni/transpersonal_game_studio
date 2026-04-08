#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // HERBÍVOROS PEQUENOS (domesticáveis)
    Compsognathus,
    Gallimimus,
    Parasaurolophus,
    Triceratops_Juvenile,
    
    // HERBÍVOROS GRANDES (neutros)
    Brontosaurus,
    Triceratops_Adult,
    Stegosaurus,
    Ankylosaurus,
    
    // CARNÍVOROS PEQUENOS (ameaça baixa)
    Dilophosaurus,
    Oviraptor,
    
    // CARNÍVOROS MÉDIOS (ameaça alta)
    Utahraptor,
    Allosaurus,
    
    // CARNÍVOROS GRANDES (ameaça extrema)
    Tyrannosaurus,
    Spinosaurus,
    Giganotosaurus
};

UENUM(BlueprintType)
enum class EDinosaurMoodState : uint8
{
    Calm,           // Estado neutro
    Curious,        // Investigando algo
    Feeding,        // Comendo
    Drinking,       // Bebendo água
    Resting,        // Descansando
    Socializing,    // Interagindo com outros da espécie
    Hunting,        // Caçando (carnívoros)
    Fleeing,        // Fugindo de predador
    Aggressive,     // Agressivo/territorial
    Mating,         // Comportamento reprodutivo
    Protecting,     // Protegendo território/filhotes
    Sick,           // Doente/ferido
    Dying           // Morrendo
};

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild,           // Completamente selvagem
    Aware,          // Consciente da presença do jogador
    Curious,        // Curioso sobre o jogador
    Tolerant,       // Tolera proximidade
    Trusting,       // Confia no jogador
    Bonded,         // Ligação emocional
    Domesticated    // Completamente domesticado
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    // Traços de personalidade (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Aggression = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Sociability = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Fearfulness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Intelligence = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Territoriality = 0.5f;

    FDinosaurPersonality()
    {
        // Valores padrão já definidos acima
    }
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Localizações importantes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> FeedingSpots;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaterSources;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SafeZones;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerZones;
    
    // Relacionamentos com outros dinossauros
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> KnownActors; // Actor -> Relationship (-1.0 hostil, 1.0 amigável)
    
    // Memória do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerRelationship = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastPlayerLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSincePlayerSeen = 0.0f;
    
    // Experiências passadas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesHuntedByPlayer = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesFedByPlayer = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesAttackedByPlayer = 0;

    FDinosaurMemory()
    {
        PlayerRelationship = 0.0f;
        LastPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        TimesHuntedByPlayer = 0;
        TimesFedByPlayer = 0;
        TimesAttackedByPlayer = 0;
    }
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    // Horários de atividades (0.0 = meia-noite, 0.5 = meio-dia, 1.0 = meia-noite)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WakeUpTime = 0.25f; // 6h
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FirstFeedingTime = 0.3f; // 7h30
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DrinkingTime = 0.4f; // 9h30
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RestTime = 0.6f; // 14h30
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SecondFeedingTime = 0.75f; // 18h
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SleepTime = 0.9f; // 21h30
    
    // Variação aleatória nas atividades (em horas)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeVariation = 1.0f;

    FDailyRoutine()
    {
        // Valores padrão já definidos acima
    }
};

/**
 * Sistema de comportamento para NPCs dinossauros
 * Cada dinossauro tem personalidade única, memória, rotinas diárias
 * e sistema de relacionamentos que evolui ao longo do tempo
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração base do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FString DinosaurName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    int32 Age = 1; // Em anos
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    bool bIsMale = true;

    // Sistemas comportamentais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDinosaurPersonality Personality;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDinosaurMemory Memory;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDailyRoutine DailyRoutine;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDinosaurMoodState CurrentMood = EDinosaurMoodState::Calm;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDomesticationLevel DomesticationLevel = EDomesticationLevel::Wild;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float Hunger = 0.0f; // 0.0 = saciado, 1.0 = faminto
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float Thirst = 0.0f; // 0.0 = hidratado, 1.0 = sedento
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float Energy = 1.0f; // 0.0 = exausto, 1.0 = descansado
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float Health = 1.0f; // 0.0 = morto, 1.0 = saudável
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float Stress = 0.0f; // 0.0 = calmo, 1.0 = extremamente estressado

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeDinosaur();
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMoodBasedOnNeeds();
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(AActor* Player, float Distance, bool bCanSeePlayer);
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToOtherDinosaur(AActor* OtherDinosaur, EDinosaurSpecies OtherSpecies);
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateDomestication(float PlayerInteractionQuality, float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetCurrentGoalLocation();
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldPerformActivity(float CurrentTime, float ActivityTime);
    
    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    bool IsSpeciesPredator(EDinosaurSpecies CheckSpecies);
    
    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    bool IsSpeciesPrey(EDinosaurSpecies CheckSpecies);
    
    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    bool CanSpeciesBecomeDomesticated(EDinosaurSpecies CheckSpecies);

private:
    // Funções internas
    void UpdateNeeds(float DeltaTime);
    void UpdateDailyRoutine();
    void UpdateMemory();
    void GenerateRandomPersonality();
    FVector FindNearestResourceLocation(const TArray<FVector>& Locations);
    
    // Timers
    float LastRoutineUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;
    
    // Cache
    AActor* CachedPlayer = nullptr;
    float LastPlayerDistance = 0.0f;
};