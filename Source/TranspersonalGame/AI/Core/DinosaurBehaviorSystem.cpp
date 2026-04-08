#include "DinosaurBehaviorSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "DinosaurMemoryComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UDinosaurBehaviorSystem::UDinosaurBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Tick a cada 0.5 segundos para performance
}

void UDinosaurBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar componentes
    MemoryComponent = GetOwner()->FindComponentByClass<UDinosaurMemoryComponent>();
    BehaviorTreeComponent = GetOwner()->FindComponentByClass<UBehaviorTreeComponent>();
    BlackboardComponent = GetOwner()->FindComponentByClass<UBlackboardComponent>();
    
    // Gerar personalidade única se não foi definida
    if (!bPersonalityGenerated)
    {
        GenerateUniquePersonality();
    }
    
    // Configurar rotinas padrão baseadas na espécie
    if (DailyRoutines.Num() == 0)
    {
        SetupDefaultRoutines();
    }
    
    // Configurar domesticação baseada na espécie
    SetupDomesticationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorSystem: %s (%s) initialized with personality"), 
           *IndividualName, *UEnum::GetValueAsString(Species));
}

void UDinosaurBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateNeeds(DeltaTime);
    UpdateRoutines(DeltaTime);
    UpdateDomestication(DeltaTime);
    EvaluateBehaviorChange();
}

void UDinosaurBehaviorSystem::GenerateUniquePersonality()
{
    // Gerar traços de personalidade baseados na espécie com variação individual
    FDinosaurPersonalityTraits BaseTraits = GetSpeciesBaseTraits(Species);
    
    // Adicionar variação aleatória (±20%)
    PersonalityTraits.Aggression = FMath::Clamp(BaseTraits.Aggression + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    PersonalityTraits.Curiosity = FMath::Clamp(BaseTraits.Curiosity + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    PersonalityTraits.Sociability = FMath::Clamp(BaseTraits.Sociability + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    PersonalityTraits.Territoriality = FMath::Clamp(BaseTraits.Territoriality + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    PersonalityTraits.Fearfulness = FMath::Clamp(BaseTraits.Fearfulness + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    PersonalityTraits.Intelligence = FMath::Clamp(BaseTraits.Intelligence + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    
    // Gerar variações físicas únicas
    PersonalityTraits.SizeModifier = FVector(
        FMath::RandRange(0.85f, 1.15f),
        FMath::RandRange(0.85f, 1.15f),
        FMath::RandRange(0.85f, 1.15f)
    );
    
    // Gerar variação de cor
    PersonalityTraits.ColorVariation = FLinearColor(
        FMath::RandRange(0.8f, 1.2f),
        FMath::RandRange(0.8f, 1.2f),
        FMath::RandRange(0.8f, 1.2f),
        1.0f
    );
    
    PersonalityTraits.VoicePitch = FMath::RandRange(0.7f, 1.3f);
    
    // Gerar nome único se não foi definido
    if (IndividualName.IsEmpty())
    {
        IndividualName = GenerateUniqueName();
    }
    
    bPersonalityGenerated = true;
    
    UE_LOG(LogTemp, Log, TEXT("Generated personality for %s: Aggr=%.2f, Cur=%.2f, Soc=%.2f, Terr=%.2f, Fear=%.2f, Int=%.2f"),
           *IndividualName, PersonalityTraits.Aggression, PersonalityTraits.Curiosity, 
           PersonalityTraits.Sociability, PersonalityTraits.Territoriality, 
           PersonalityTraits.Fearfulness, PersonalityTraits.Intelligence);
}

FDinosaurPersonalityTraits UDinosaurBehaviorSystem::GetSpeciesBaseTraits(EDinosaurSpecies InSpecies)
{
    FDinosaurPersonalityTraits BaseTraits;
    
    switch (InSpecies)
    {
        case EDinosaurSpecies::Compsognathus:
            BaseTraits.Aggression = 0.3f;
            BaseTraits.Curiosity = 0.8f;
            BaseTraits.Sociability = 0.7f;
            BaseTraits.Territoriality = 0.2f;
            BaseTraits.Fearfulness = 0.8f;
            BaseTraits.Intelligence = 0.6f;
            break;
            
        case EDinosaurSpecies::Tyrannosaurus:
            BaseTraits.Aggression = 0.9f;
            BaseTraits.Curiosity = 0.4f;
            BaseTraits.Sociability = 0.1f;
            BaseTraits.Territoriality = 0.9f;
            BaseTraits.Fearfulness = 0.1f;
            BaseTraits.Intelligence = 0.7f;
            break;
            
        case EDinosaurSpecies::Triceratops_Adult:
            BaseTraits.Aggression = 0.4f;
            BaseTraits.Curiosity = 0.3f;
            BaseTraits.Sociability = 0.6f;
            BaseTraits.Territoriality = 0.5f;
            BaseTraits.Fearfulness = 0.4f;
            BaseTraits.Intelligence = 0.5f;
            break;
            
        // Adicionar outros casos conforme necessário
        default:
            BaseTraits.Aggression = 0.5f;
            BaseTraits.Curiosity = 0.5f;
            BaseTraits.Sociability = 0.5f;
            BaseTraits.Territoriality = 0.5f;
            BaseTraits.Fearfulness = 0.5f;
            BaseTraits.Intelligence = 0.5f;
            break;
    }
    
    return BaseTraits;
}

void UDinosaurBehaviorSystem::SetupDefaultRoutines()
{
    // Rotinas baseadas na espécie e tipo de dinossauro
    switch (ThreatLevel)
    {
        case EDinosaurThreatLevel::Passive:
            SetupHerbivoreRoutines();
            break;
        case EDinosaurThreatLevel::Cautious:
            SetupLargeHerbivoreRoutines();
            break;
        case EDinosaurThreatLevel::Opportunist:
        case EDinosaurThreatLevel::Aggressive:
        case EDinosaurThreatLevel::Apex:
            SetupCarnivoreRoutines();
            break;
    }
}

void UDinosaurBehaviorSystem::SetupHerbivoreRoutines()
{
    // Manhã: Forragear
    FDinosaurRoutine MorningForage;
    MorningForage.PrimaryActivity = EDinosaurBehaviorState::Foraging;
    MorningForage.StartTime = 6.0f;
    MorningForage.Duration = 3.0f;
    MorningForage.Priority = 0.8f;
    DailyRoutines.Add(MorningForage);
    
    // Meio-dia: Descansar
    FDinosaurRoutine MiddayRest;
    MiddayRest.PrimaryActivity = EDinosaurBehaviorState::Resting;
    MiddayRest.StartTime = 12.0f;
    MiddayRest.Duration = 2.0f;
    MiddayRest.Priority = 0.6f;
    DailyRoutines.Add(MiddayRest);
    
    // Tarde: Socializar
    FDinosaurRoutine AfternoonSocial;
    AfternoonSocial.PrimaryActivity = EDinosaurBehaviorState::Socializing;
    AfternoonSocial.StartTime = 15.0f;
    AfternoonSocial.Duration = 2.0f;
    AfternoonSocial.Priority = 0.5f;
    DailyRoutines.Add(AfternoonSocial);
    
    // Entardecer: Beber água
    FDinosaurRoutine EveningDrink;
    EveningDrink.PrimaryActivity = EDinosaurBehaviorState::Drinking;
    EveningDrink.StartTime = 18.0f;
    EveningDrink.Duration = 1.0f;
    EveningDrink.Priority = 0.9f;
    DailyRoutines.Add(EveningDrink);
}

void UDinosaurBehaviorSystem::SetupCarnivoreRoutines()
{
    // Madrugada: Caçar
    FDinosaurRoutine DawnHunt;
    DawnHunt.PrimaryActivity = EDinosaurBehaviorState::Hunting;
    DawnHunt.StartTime = 5.0f;
    DawnHunt.Duration = 2.0f;
    DawnHunt.Priority = 0.9f;
    DailyRoutines.Add(DawnHunt);
    
    // Manhã: Patrulhar território
    FDinosaurRoutine MorningPatrol;
    MorningPatrol.PrimaryActivity = EDinosaurBehaviorState::Territorial;
    MorningPatrol.StartTime = 8.0f;
    MorningPatrol.Duration = 2.0f;
    MorningPatrol.Priority = 0.7f;
    DailyRoutines.Add(MorningPatrol);
    
    // Tarde: Descansar
    FDinosaurRoutine AfternoonRest;
    AfternoonRest.PrimaryActivity = EDinosaurBehaviorState::Resting;
    AfternoonRest.StartTime = 14.0f;
    AfternoonRest.Duration = 4.0f;
    AfternoonRest.Priority = 0.6f;
    DailyRoutines.Add(AfternoonRest);
    
    // Entardecer: Caçar novamente
    FDinosaurRoutine EveningHunt;
    EveningHunt.PrimaryActivity = EDinosaurBehaviorState::Hunting;
    EveningHunt.StartTime = 19.0f;
    EveningHunt.Duration = 2.0f;
    EveningHunt.Priority = 0.8f;
    DailyRoutines.Add(EveningHunt);
}

void UDinosaurBehaviorSystem::SetupDomesticationSettings()
{
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Triceratops_Juvenile:
            bCanBeDomesticated = true;
            TrustDecayRate = 0.5f; // Decay lento
            break;
        default:
            bCanBeDomesticated = false;
            TrustDecayRate = 2.0f; // Decay rápido
            break;
    }
}

FString UDinosaurBehaviorSystem::GenerateUniqueName()
{
    TArray<FString> NamePrefixes = {TEXT("Alpha"), TEXT("Beta"), TEXT("Gamma"), TEXT("Delta"), TEXT("Epsilon"), 
                                   TEXT("Zeta"), TEXT("Theta"), TEXT("Kappa"), TEXT("Lambda"), TEXT("Sigma")};
    
    FString SpeciesName = UEnum::GetValueAsString(Species).Replace(TEXT("EDinosaurSpecies::"), TEXT(""));
    FString Prefix = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num() - 1)];
    int32 Number = FMath::RandRange(1, 999);
    
    return FString::Printf(TEXT("%s-%s-%03d"), *Prefix, *SpeciesName, Number);
}

void UDinosaurBehaviorSystem::UpdateNeeds(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastNeedsUpdate < 1.0f) return; // Update a cada segundo
    
    LastNeedsUpdate = CurrentTime;
    
    // Decaimento natural das necessidades
    Hunger = FMath::Clamp(Hunger - (DeltaTime * 0.5f), 0.0f, 100.0f);
    Thirst = FMath::Clamp(Thirst - (DeltaTime * 0.8f), 0.0f, 100.0f);
    Energy = FMath::Clamp(Energy - (DeltaTime * 0.3f), 0.0f, 100.0f);
    
    // Decaimento social baseado na sociabilidade
    float SocialDecay = (1.0f - PersonalityTraits.Sociability) * DeltaTime * 0.2f;
    Social = FMath::Clamp(Social - SocialDecay, 0.0f, 100.0f);
    
    // Atualizar Blackboard se disponível
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Thirst"), Thirst);
        BlackboardComponent->SetValueAsFloat(TEXT("Energy"), Energy);
        BlackboardComponent->SetValueAsFloat(TEXT("Social"), Social);
    }
}

void UDinosaurBehaviorSystem::UpdateRoutines(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoutineCheck < 5.0f) return; // Check a cada 5 segundos
    
    LastRoutineCheck = CurrentTime;
    
    FDinosaurRoutine* ActiveRoutine = FindActiveRoutine();
    if (ActiveRoutine)
    {
        // Se há uma rotina ativa e não estamos seguindo ela, mudar comportamento
        if (CurrentBehaviorState != ActiveRoutine->PrimaryActivity)
        {
            SetBehaviorState(ActiveRoutine->PrimaryActivity);
            UE_LOG(LogTemp, Log, TEXT("%s: Switching to routine activity: %s"), 
                   *IndividualName, *UEnum::GetValueAsString(ActiveRoutine->PrimaryActivity));
        }
    }
}

void UDinosaurBehaviorSystem::UpdateDomestication(float DeltaTime)
{
    if (!bCanBeDomesticated) return;
    
    // Decaimento natural da confiança
    if (TrustedHuman && DomesticationLevel > 0.0f)
    {
        float DecayAmount = TrustDecayRate * DeltaTime / 86400.0f; // Por dia
        DomesticationLevel = FMath::Clamp(DomesticationLevel - DecayAmount, 0.0f, 100.0f);
        
        // Se a confiança cai muito, remover humano confiável
        if (DomesticationLevel < 10.0f)
        {
            TrustedHuman = nullptr;
        }
    }
    
    // Atualizar Blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("DomesticationLevel"), DomesticationLevel);
        BlackboardComponent->SetValueAsObject(TEXT("TrustedHuman"), TrustedHuman);
    }
}

void UDinosaurBehaviorSystem::EvaluateBehaviorChange()
{
    // Avaliar se deve mudar comportamento baseado em necessidades urgentes
    if (Thirst < 20.0f && CurrentBehaviorState != EDinosaurBehaviorState::Drinking)
    {
        SetBehaviorState(EDinosaurBehaviorState::Drinking);
        return;
    }
    
    if (Hunger < 15.0f && CurrentBehaviorState != EDinosaurBehaviorState::Foraging && 
        CurrentBehaviorState != EDinosaurBehaviorState::Hunting)
    {
        EDinosaurBehaviorState FoodBehavior = (ThreatLevel >= EDinosaurThreatLevel::Opportunist) ? 
                                             EDinosaurBehaviorState::Hunting : EDinosaurBehaviorState::Foraging;
        SetBehaviorState(FoodBehavior);
        return;
    }
    
    if (Energy < 10.0f && CurrentBehaviorState != EDinosaurBehaviorState::Resting)
    {
        SetBehaviorState(EDinosaurBehaviorState::Resting);
        return;
    }
}

FDinosaurRoutine* UDinosaurBehaviorSystem::FindActiveRoutine()
{
    float CurrentTimeOfDay = GetCurrentTimeOfDay();
    
    for (FDinosaurRoutine& Routine : DailyRoutines)
    {
        float EndTime = Routine.StartTime + Routine.Duration;
        
        // Handle routines that cross midnight
        if (EndTime > 24.0f)
        {
            if (CurrentTimeOfDay >= Routine.StartTime || CurrentTimeOfDay <= (EndTime - 24.0f))
            {
                return &Routine;
            }
        }
        else
        {
            if (CurrentTimeOfDay >= Routine.StartTime && CurrentTimeOfDay <= EndTime)
            {
                return &Routine;
            }
        }
    }
    
    return nullptr;
}

float UDinosaurBehaviorSystem::GetCurrentTimeOfDay() const
{
    // Assumindo um ciclo de 24 horas baseado no tempo do mundo
    // Isso deve ser integrado com o sistema de dia/noite do jogo
    float WorldTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutos = 1 dia no jogo
    float TimeInDay = FMath::Fmod(WorldTime, DayLength);
    return (TimeInDay / DayLength) * 24.0f;
}

void UDinosaurBehaviorSystem::SetBehaviorState(EDinosaurBehaviorState NewState, AActor* Target)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        CurrentTarget = Target;
        CurrentStateTime = 0.0f;
        
        // Atualizar Blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
            BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), Target);
        }
        
        UE_LOG(LogTemp, Log, TEXT("%s: Behavior changed from %s to %s"), 
               *IndividualName, 
               *UEnum::GetValueAsString(PreviousState), 
               *UEnum::GetValueAsString(NewState));
    }
}

void UDinosaurBehaviorSystem::AddTrust(float Amount, AActor* Human)
{
    if (!bCanBeDomesticated || !Human) return;
    
    // Modificar baseado na personalidade
    float PersonalityModifier = (PersonalityTraits.Fearfulness * -0.5f) + (PersonalityTraits.Intelligence * 0.3f) + 0.5f;
    float AdjustedAmount = Amount * PersonalityModifier;
    
    DomesticationLevel = FMath::Clamp(DomesticationLevel + AdjustedAmount, 0.0f, 100.0f);
    
    // Definir humano confiável se confiança é alta o suficiente
    if (DomesticationLevel > 30.0f)
    {
        TrustedHuman = Human;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Trust increased by %.2f (total: %.2f) towards %s"), 
           *IndividualName, AdjustedAmount, DomesticationLevel, 
           Human ? *Human->GetName() : TEXT("Unknown"));
}

void UDinosaurBehaviorSystem::RemoveTrust(float Amount)
{
    DomesticationLevel = FMath::Clamp(DomesticationLevel - Amount, 0.0f, 100.0f);
    
    if (DomesticationLevel < 10.0f)
    {
        TrustedHuman = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Trust decreased by %.2f (total: %.2f)"), 
           *IndividualName, Amount, DomesticationLevel);
}

bool UDinosaurBehaviorSystem::CanBeApproached(AActor* Human) const
{
    if (!bCanBeDomesticated) return false;
    
    // Se é o humano confiável, sempre pode aproximar
    if (TrustedHuman == Human) return true;
    
    // Baseado na personalidade e nível de domesticação
    float ApproachThreshold = PersonalityTraits.Fearfulness * 50.0f; // 0-50
    return DomesticationLevel > ApproachThreshold;
}

float UDinosaurBehaviorSystem::GetPersonalityTrait(const FString& TraitName) const
{
    if (TraitName == TEXT("Aggression")) return PersonalityTraits.Aggression;
    if (TraitName == TEXT("Curiosity")) return PersonalityTraits.Curiosity;
    if (TraitName == TEXT("Sociability")) return PersonalityTraits.Sociability;
    if (TraitName == TEXT("Territoriality")) return PersonalityTraits.Territoriality;
    if (TraitName == TEXT("Fearfulness")) return PersonalityTraits.Fearfulness;
    if (TraitName == TEXT("Intelligence")) return PersonalityTraits.Intelligence;
    
    return 0.5f; // Valor padrão
}